/////////////////////////////////////////////////////////////////////////
// MainWindow.Xaml.cs - Console App that provides a GUI to the client  //
// ver 1.0                                                             //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018      //
/////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
*  - This package provides a WPF-based GUI to connect to the Repo Server. 
*  - Following tabs are provided as a part of the GUI -
*  - Connect : Takes Client Host and Port as input and Connects to server at port 8080, Localhost 
*  - Checkin : Allows the user to checkin files. Provides user with following options:
*       - Allows user to Browse the file system to select the file
*       - Allows user to Specify the description of the file
*       - Allows user to Specify the checkin status of the file
*       - Allows user to Add dependency files from the File Browse window
*       - Allows user to Add Categories for the file
*       - User specifies the namespace, which serves as a directory within the root directory ../Storage
* - Checkout: User can browse through the respository on server and double click the file to checkout at client repo.
* - Browse and View Full File Feature:
*       - This Tab allows the user to browse through the respository on server and double click the file to extract it.
*       - On Double click the extracted file is Launched on a separate Full View Window, The user can view the complete file.
* - View metadata: This Tab allows the user to browse through the files on the repo and on a double click , the server returns the 
*           metadata for thr file.
* - MainWindow.cs has private event handlers, dispatcher and message processor
* - MainWindow.cs is a partial public class
*  Message handling runs on a child thread, so the Server main thread is free to do
*  any necessary background processing (none, so far).
* Required Files:
* ---------------
* Mainwindow.xaml, MainWindow.xaml.cs
* Translater.dll
* 
* Maintenance History:
* --------------------
* ver 1.0 : 9 April 2018
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using MsgPassingCommunication;



namespace WPFClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private Stack<string> pathStack_ = new Stack<string>();
        private Stack<string> pathStack_checkout = new Stack<string>();
        private Stack<string> pathStack_meta = new Stack<string>();
        private Stack<string> pathStack_checkin = new Stack<string>();
        private Translater translater;
        private CsEndPoint endPoint_;
        private Thread rcvThrd = null;
        private Dictionary<string, Action<CsMessage>> dispatcher_
          = new Dictionary<string, Action<CsMessage>>();
        private CsEndPoint serverEndPoint;
        private string fileSelected;
        private string statusSelected = "Open";
        private List<CsMessage> fileList = new List<CsMessage>();
        //----< process incoming messages on child thread >----------------

        private void processMessages()
        {
            ThreadStart thrdProc = () =>
            {
                while (true)
                {
                    CsMessage msg = translater.getMessage();
                    string msgId = msg.value("command");
             
                    if (dispatcher_.ContainsKey(msgId))
                        dispatcher_[msgId].Invoke(msg);

                    if (msgId == "clientQuit")
                    {
                        translater.endConnection();
                        break;
                    }
                }
            };
            rcvThrd = new Thread(thrdProc);
            rcvThrd.IsBackground = true;
            rcvThrd.Start();
        }
        //----< add client processes to the dispatcher dictionery >----------------
        private void addClientProc(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }

        //----< function dispatched by child thread to main thread >-------
        private void clearDirs(string tab)
        {
            if (tab == "browse")
            {
                DirList.Items.Clear();
            }
            else if (tab == "checkout")
            {
                Check_DirList.Items.Clear();
            }
            else if (tab == "meta")
            {
                Meta_DirList.Items.Clear();
            }
            else if(tab == "checkin")
            {
                Checkin_DirList.Items.Clear();
            }

        }
        //----< function dispatched by child thread to main thread >-------
        private void addDir(string dir, string tab)
        {
            if (tab == "browse")
            {
                DirList.Items.Add(dir);
            }
            else if (tab == "checkout")
            {
                Check_DirList.Items.Add(dir);
            }
            else if (tab == "meta")
            {
                Meta_DirList.Items.Add(dir);
            }
            else if(tab == "checkin")
            {
                Checkin_DirList.Items.Add(dir);
            }

        }
        //----< function dispatched by child thread to main thread >-------

        private void insertParent(string tab)
        {
            if (tab == "browse")
            {
                DirList.Items.Insert(0, "..");
            }
            else if (tab == "checkout")
            {
                Check_DirList.Items.Insert(0, "..");
            }
            else if (tab == "meta")
            {
                Meta_DirList.Items.Insert(0, "..");
            }
            else if(tab == "checkin")
            {
                Checkin_DirList.Items.Insert(0, "..");
            }

        }
        //----< function dispatched by child thread to main thread >-------

        private void clearFiles(string tab)
        {
            if (tab == "browse")
            {
                FileList.Items.Clear();
            }
            else if (tab == "checkout")
            {
                Check_FileList.Items.Clear();
            }
            else if (tab == "meta")
            {
                Meta_FileList.Items.Clear();
            }
            else if(tab == "checkin")
            {
                Checkin_FileList.Items.Clear();
            }
        }
        //----< function dispatched by child thread to main thread >-------

        private void addFile(string file, string tab)
        {
            if (tab == "browse")
            {
                FileList.Items.Add(file);
            }
            else if (tab == "checkout")
            {
                Check_FileList.Items.Add(file);
            }
            else if (tab == "meta")
            {
                Meta_FileList.Items.Add(file);
            }
            else if(tab == "checkin")
            {
                CheckBox checkbox = new CheckBox();
                checkbox.Content = file;
                Checkin_FileList.Items.Add(checkbox);
            }
        }
        //----< load getDirs processing into dispatcher dictionary >-------
        private void DispatcherLoadGetDirs()
        {
            Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
            {
                Action clrDirs = () =>
                {
                    clearDirs(rcvMsg.value("tab"));
                };
                Dispatcher.Invoke(clrDirs, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("dir"))
                    {
                        Action<string> doDir = (string dir) =>
                        {
                            addDir(dir, rcvMsg.value("tab"));
                            statusBarText.Text = "Directories fetched";
                        };
                        Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
                    }
                }
                Action insertUp = () =>
                {
                    insertParent(rcvMsg.value("tab"));
                };
                Dispatcher.Invoke(insertUp, new Object[] { });
            };
            addClientProc("getDirs", getDirs);
        }
        //----< load getFiles processing into dispatcher dictionary >------
        private void DispatcherLoadGetFiles()
        {
            Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
            {
                Action clrFiles = () =>
                {
                    clearFiles(rcvMsg.value("tab"));
                };
                Dispatcher.Invoke(clrFiles, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> doFile = (string file) =>
                        {
                            addFile(file, rcvMsg.value("tab"));
                            statusBarText.Text = "Files fetched";
                        };
                        Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("getFiles", getFiles);
        }

        //----< Connect to the server >------
        private void DispatcherConnectionMessage()
        {

            Action<CsMessage> connectmessage = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    string content = rcvMsg.value("content");
                    server_reply.Text += "\n" + content;
                    statusBarText.Text = "Connected";
                };

                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });

            };
            addClientProc("connectionReq", connectmessage);
        }

        //----< Disconnect from the server >------
        private void DispatcherDisconnectMessage()
        {
            Action<CsMessage> disconnectmessage = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    string content = rcvMsg.value("content");
                    server_reply.Text += "\n" + content;
                    statusBarText.Text = "Disconnected";
                    server_reply.Text = "Disconnected";
                    pathStack_.Clear();
                    pathStack_checkin.Clear();
                    pathStack_checkout.Clear();
                    pathStack_meta.Clear();
                    Brose_file_Path.Clear(); description_box.Clear(); namespace_box.Clear(); Category_Text.Clear();
                    Categories_list.Items.Clear();depen_list.Items.Clear(); Checkin_Console.Clear();
                    Checkout_Console.Clear(); Browse_Console.Clear(); Meta_Console.Clear(); Category_Query.Clear(); Dep_Query.Clear();
                    clearDirs("browse"); clearDirs("checkin"); clearDirs("checkout"); clearDirs("meta");
                    clearFiles("browse"); clearFiles("checkin"); clearFiles("checkout"); clearFiles("meta");
                    status_box.Text = "Open";

                };
                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });

            };
            addClientProc("clientQuit", disconnectmessage);
        }
        //----< Query Exec Response from Server >------
        private void DispatcherQueryMessage()
        {
            Action<CsMessage> browse = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    string content = rcvMsg.value("content");
                    if (rcvMsg.value("status") == "success")
                    {
                        Browse_Console.Text += "\n------------------------------ "+ content + " --------------------------------------";
                        Browse_Console.Text += "\nQuery Result : \n" + rcvMsg.value("fileList") + "\n";
                        fetchQueryFile(rcvMsg.value("key"), rcvMsg.value("filename"));
                    }
                    else
                    {
                        Browse_Console.Text += "\n------------------------------ " + content + " --------------------------------------";
                        Browse_Console.Text += "\n" + "No Result of the query";
                    }
                 
                      statusBarText.Text = "Query Execution completed";
                };

                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });

            };
            addClientProc("query_exec_response", browse);
        }

        //----< get files Response from Server >------
        private void DispatcherGetFilesMessage()
        {
            Action<CsMessage> getFilesFromServ = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    string content = rcvMsg.value("content");
       
                    if (rcvMsg.value("tab") == "browse")
                    {
                        Browse_Console.Text += "\n" + content;
                    }
                    else
                    {
                        Checkout_Console.Text += "\n" + content;
                    }
                    statusBarText.Text = "Fetching file..";
                    fetchFiles(rcvMsg.value("allFiles"),rcvMsg.value("tab"),rcvMsg.value("key"));
                };

                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });

            };
            addClientProc("getFiles_response", getFilesFromServ);
        }

        //----< Browse File/ Extract file Response from Server >------
        private void DispatcherBrowseMessage()
        {
            Action<CsMessage> browse = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    statusBarText.Text = "Fetching file..";
                };

                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });

            };
            addClientProc("extractFiles_response", browse);
        }
        //----< Launch File Viewer to view browsed file >------
        private void DispatcherReadFiles()
        {
            Action<CsMessage> browse = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    string content = rcvMsg.value("content");
                    if (rcvMsg.value("tab") == "browse")
                    {
                        string text = System.IO.File.ReadAllText("../../../client_stage/" + rcvMsg.value("filenames"));// append path and filename in the response
                        Browse_Console.Text += "\n" + content;
                        Browse_Console.Text += "\n" + rcvMsg.value("filenames") + " checked out at location client_stage";
                        Browse_Console.Text += "\n MetaData for the file : ";
                        Browse_Console.Text += "\n Filename: " + rcvMsg.value("filenames");
                        Browse_Console.Text += "\n Description: " + rcvMsg.value("description");
                        Browse_Console.Text += "\n File owner: " + rcvMsg.value("Owner");
                        Browse_Console.Text += "\n Status: " + rcvMsg.value("Status");
                        Browse_Console.Text += "\n DateTime: " + rcvMsg.value("DateTime");
                        Browse_Console.Text += "\n Categories: " + rcvMsg.value("Categories");
                        Browse_Console.Text += "\n Children/Its Dependencies: " + rcvMsg.value("Children");
                        Browse_Console.Text += "\n";
                        launchFileWindow(text, rcvMsg.value("filenames"));
                    }
                    else
                    {
                        Checkout_Console.Text += "\n" + rcvMsg.value("filenames") + " checked out at location client_stage";
                        Checkout_Console.Text += "\n";
                    }
                    statusBarText.Text = "file fetch completed.";
                };
                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });
            };
            addClientProc("file_read", browse);
        }

        //----<Fetch the file from query list from server >----
        // format -> ::namespace2::file2.cpp.2 , file2.cpp.2
        private void fetchQueryFile(string key, string filename) 
        {
            statusBarText.Text = "Waiting for file to browse";
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "extractFiles");
            msg.add("tab", "browse");

            string[] fileSplit = filename.Split('.');
            if (fileSplit.Length < 3)
                return;

            string[] keySplit = key.Split('.');
            if (key.Length < 3)
                return;

            string searchFile = keySplit[0] + "." + keySplit[1];

            msg.add("filename", fileSplit[0] + "." + fileSplit[1]);
            msg.add("searchFile", searchFile);
            msg.add("version", fileSplit[2]);
            msg.add("sender", "client");
            translater.postMessage(msg);
        }

        //----<Fetch the file list from server >----
        private void fetchFiles(string allFiles , string tab , string key)
        {
            string[] files = allFiles.Split(' ');
           
            for (int i = 0; i < files.Length ; i++){
                statusBarText.Text = "Fetching " + files[i];
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "getFilesFromServ");
                msg.add("tab", tab);
                msg.add("filename", files[i]);
                msg.add("key",key);
                msg.add("sender", "client");

                translater.postMessage(msg);
            }
        }

        //----< Meta Data Response from the server >------
        private void DispatcherViewMetaData()
        {
            Action<CsMessage> metaData = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    string content = rcvMsg.value("content");

                    Meta_Console.Text += "\n" + content;
                    Meta_Console.Text += "\n Filename: " + rcvMsg.value("filenames");
                    Meta_Console.Text += "\n Description: " + rcvMsg.value("description");
                    Meta_Console.Text += "\n File owner: " + rcvMsg.value("Owner");
                    Meta_Console.Text += "\n Status: " + rcvMsg.value("Status");
                    Meta_Console.Text += "\n DateTime: " + rcvMsg.value("DateTime");
                    Meta_Console.Text += "\n Categories: " + rcvMsg.value("Categories");
                    Meta_Console.Text += "\n Children/Its Dependencies: " + rcvMsg.value("Children");
                    Meta_Console.Text += "\n";
                    statusBarText.Text = "Fetched Metadata";
                };
               
                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });
            };
            addClientProc("fetchMetaData_response", metaData);

        }
        //----< Checkin progress Response from the server >------
        private void DispatcherCheckinProgress()
        {
            Action<CsMessage> checkinDone = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    string content = rcvMsg.value("content");
                    Checkin_Console.Text += "\n"+content;
                    statusBarText.Text = "file checkin in Progress";
                };
         
                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });
            };
            addClientProc("checkin_progress", checkinDone);
        }

        //----< Checkin  Response from the server >------
        private void DispatcherCheckin()
        {
            Action<CsMessage> checkinDone = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    description_box.Clear(); namespace_box.Clear(); Category_Text.Clear();
                    Categories_list.Items.Clear(); depen_list.Items.Clear(); file_list.Items.Clear(); owner_box.Clear(); Brose_file_Path.Clear();
                    string content = rcvMsg.value("content");
                    Checkin_Console.Text += "\n"+content;
                    if(rcvMsg.value("stat") == "pass")
                    {
                        string path = rcvMsg.value("namespaces");
                        path = path.Substring(2, path.Length - 4);
                        refreshWindow("root/" + path);
                        statusBarText.Text = "file checkin completed.";
                    }
                    else
                    {
                        statusBarText.Text = "file checkin Failed.";
                    }
                   
                };

                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });
            };
            addClientProc("checkin_response", checkinDone);
        }
        //----< dispatcher on closing event  >---------------------------
        private void DispatcherClosed()
        {
            Action<CsMessage> checkinDone = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> getValue = (CsMessage attribute) =>
                {
                    Console.WriteLine("System ShutDown Completed");
                };

                Dispatcher.Invoke(getValue, new Object[] { rcvMsg });
            };
            addClientProc("close_system_response", checkinDone);
        }

        //----< load all dispatcher processing  >---------------------------
        private void loadDispatcher()
        {
            DispatcherConnectionMessage();
            DispatcherDisconnectMessage();
            DispatcherLoadGetDirs();
            DispatcherLoadGetFiles();
            DispatcherBrowseMessage();
            DispatcherReadFiles();
            DispatcherCheckinProgress();
            DispatcherCheckin();
            DispatcherViewMetaData();
            DispatcherGetFilesMessage();
            DispatcherQueryMessage();
            DispatcherClosed();
        }
        //----< On Window Load , Load the dispatchers >------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            loadDispatcher();
             string[] args = Environment.GetCommandLineArgs();
              if (args.Length == 3)
              {
                  automationStub(args[2]);
              }
              else { automationStub(); }
        }

        //----< Connect to the server Request>------
        private void connectServer(object sender, RoutedEventArgs e)
        {
            endPoint_ = new CsEndPoint();
            endPoint_.machineAddress = Machine_Name.Text;
            endPoint_.port = Int32.Parse(Machine_Port.Text);
            translater = new Translater();
            translater.listen(endPoint_);
            statusBarText.Text = "Connecting to server..";
            processMessages();
            loadDispatcher();
            serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "connectionRequest");
            server_reply.Text = "Attempting to connect to server at port 8080..";
            dis_btn.IsEnabled = true;
            con_btn.IsEnabled = false;
            checkin_tab.IsEnabled = true;
            checkout_tab.IsEnabled = true;
            browse_tab.IsEnabled = true;
            view_meta_tab.IsEnabled = true;
            translater.postMessage(msg);
            loadDirectories(sender, e);
            loadDirectoriesCheckout(sender, e);
            loadDirectoriesMeta(sender, e);
            loadDirectoriesCheckin(sender,e);

        }
        //----< Disconnect from the server  Request>------
        private void disconnectServer(object sender, RoutedEventArgs e)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(endPoint_));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "clientQuit");
            msg.add("content", "Disconnected From Server");
            server_reply.Text = "Disconnecting from server";
            statusBarText.Text = "Disconnecting from server..";
            pathStack_.Clear();
            pathStack_checkin.Clear();
            pathStack_checkout.Clear();
            pathStack_meta.Clear();
            clearDirs("browse"); clearDirs("checkin"); clearDirs("checkout"); clearDirs("meta");
            clearFiles("browse"); clearFiles("checkin"); clearFiles("checkout"); clearFiles("meta");
            con_btn.IsEnabled = true;
            dis_btn.IsEnabled = false;
            checkin_tab.IsEnabled = false;
            checkout_tab.IsEnabled = false;
            browse_tab.IsEnabled = false;
            view_meta_tab.IsEnabled = false;
            translater.postMessage(msg);
            statusBarText.Text = "Disconnected";
            server_reply.Text = "Disconnected";

        }

        //----< Load Directories for browse tab>------
        private void loadDirectories(object sender, RoutedEventArgs e)
        {
            statusBarText.Text = "Loading Directroies and files for browse tab";
            PathTextBlock.Text = "root";
            pathStack_.Push("../root");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("tab", "browse");
            msg.add("command", "getDirs");
            msg.add("path", pathStack_.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< Load Directories for checkout tab>------
        private void loadDirectoriesCheckout(object sender, RoutedEventArgs e)
        {
            statusBarText.Text = "Loading Directroies and files for checkout tab";
            Check_PathTextBlock.Text = "root";
            pathStack_checkout.Push("../root");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("tab", "checkout");
            msg.add("path", pathStack_checkout.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< Load Directories for Meta Data tab>------
        private void loadDirectoriesMeta(object sender, RoutedEventArgs e)
        {
            statusBarText.Text = "Loading Directroies and files for metadata tab";
            Meta_PathTextBlock.Text = "root";
            pathStack_meta.Push("../root");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("tab", "meta");
            msg.add("path", pathStack_meta.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< Load Directories for Checkin tab>------
        private void loadDirectoriesCheckin(object sender, RoutedEventArgs e)
        {
            statusBarText.Text = "Loading Directroies and files for Checkin Tab";
            Checkin_PathTextBlock.Text = "root";
            pathStack_checkin.Push("../root");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("tab", "checkin");
            msg.add("path", pathStack_checkin.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< strip off name of first part of path >---------------------
        private string removeFirstDir(string path)
        {
            string modifiedPath = path;
            int pos = path.IndexOf("/");
            modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
            return modifiedPath;
        }

        //----< respond to mouse double-click on dir name for browse tab>----------------
        private void DirList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string selectedDir = (string)DirList.SelectedItem;
            string path;
            statusBarText.Text = "Loading Directroies and files for Browse";
            if (selectedDir == "..")
            {
                if (pathStack_.Count > 1)  // don't pop off "Storage"
                    pathStack_.Pop();
                else
                    return;
            }
            else
            {
                path = pathStack_.Peek() + "/" + selectedDir;
                pathStack_.Push(path);
            }
            PathTextBlock.Text = removeFirstDir(pathStack_.Peek());
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("tab", "browse");
            msg.add("command", "getDirs");
            msg.add("path", pathStack_.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< respond to mouse double-click on dir name for checkout tab>----------------
        private void Check_DirList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string selectedDir = (string)Check_DirList.SelectedItem;
            statusBarText.Text = "Fetching Files...";
            string path;
            if (selectedDir == "..")
            {
                if (pathStack_checkout.Count > 1)  // don't pop off "Storage"
                    pathStack_checkout.Pop();
                else
                    return;
            }
            else
            {
                path = pathStack_checkout.Peek() + "/" + selectedDir;
                pathStack_checkout.Push(path);
            }
            Check_PathTextBlock.Text = removeFirstDir(pathStack_checkout.Peek());
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("tab", "checkout");
            msg.add("command", "getDirs");
            msg.add("path", pathStack_checkout.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< respond to mouse double-click on dir name for meta data tab >----------------
        private void Meta_DirList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string selectedDir = (string)Meta_DirList.SelectedItem;
            statusBarText.Text = "Fetching Files...";
            string path;
            if (selectedDir == "..")
            {
                if (pathStack_meta.Count > 1)  // don't pop off "Storage"
                    pathStack_meta.Pop();
                else
                    return;
            }
            else
            {
                path = pathStack_meta.Peek() + "/" + selectedDir;
                pathStack_meta.Push(path);
            }
            Meta_PathTextBlock.Text = removeFirstDir(pathStack_meta.Peek());
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("tab", "meta");
            msg.add("command", "getDirs");
            msg.add("path", pathStack_meta.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< respond to mouse double-click on dir name for checkin tab >----------------
        private void Checkin_DirList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string selectedDir = (string)Checkin_DirList.SelectedItem;
            statusBarText.Text = "Fetching Files...";
            string path;
            if (selectedDir == "..")
            {
                if (pathStack_checkin.Count > 1)  // don't pop off "Storage"
                    pathStack_checkin.Pop();
                else
                    return;
            }
            else
            {
                path = pathStack_checkin.Peek() + "/" + selectedDir;
                pathStack_checkin.Push(path);
            }
            Checkin_PathTextBlock.Text = removeFirstDir(pathStack_checkin.Peek());
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("tab", "checkin");
            msg.add("command", "getDirs");
            msg.add("path", pathStack_checkin.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< respond to mouse double-click on file name for browse tab >----------------
        private void FileList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            statusBarText.Text = "Waiting for file to browse";
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "extractFiles");
            msg.add("tab", "browse");
            string nameSp = PathTextBlock.Text.Split('/')[PathTextBlock.Text.Split('/').Length - 1];
            if (nameSp == "root")
                nameSp = "";
            string selectedDir = (string)FileList.SelectedItem;
            string[] fileSplit = selectedDir.Split('.');
            if (fileSplit.Length < 3)
                return;
            string searchFile = "::" + nameSp + "::" + fileSplit[0] + "." + fileSplit[1];

            msg.add("namespaces", nameSp);
            msg.add("filename", fileSplit[0] + "." + fileSplit[1]);
            msg.add("searchFile", searchFile);
            msg.add("version", fileSplit[2]);
            msg.add("sender", "client");
            translater.postMessage(msg);
        }

        //----< respond to mouse double-click on file name for checkout tab >----------------
        private void Check_FileList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            statusBarText.Text = "Waiting for file to checkout";
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "extractFiles");
            msg.add("tab", "checkout");
            string nameSp = Check_PathTextBlock.Text.Split('/')[Check_PathTextBlock.Text.Split('/').Length - 1];
            if (nameSp == "root")
                nameSp = "";
            string selectedDir = (string)Check_FileList.SelectedItem;
            string[] fileSplit = selectedDir.Split('.');
            if (fileSplit.Length < 3)
                return;
            string searchFile = "::" + nameSp + "::" + fileSplit[0] + "." + fileSplit[1];
 
            msg.add("namespaces", nameSp);
            msg.add("filename", fileSplit[0]+"."+fileSplit[1]);
            msg.add("searchFile", searchFile);
            msg.add("version",fileSplit[2]);
            msg.add("sender", "client");
            
            translater.postMessage(msg);
        }

        //----< respond to mouse double-click on file name for meta data tab >----------------
        private void Meta_FileList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            statusBarText.Text = "Waiting for file";
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "fetchMetaData");
            msg.add("tab", "meta");
            string nameSp = Meta_PathTextBlock.Text.Split('/')[Meta_PathTextBlock.Text.Split('/').Length - 1];
            if (nameSp == "root")
                nameSp = "";
            string selectedDir = (string)Meta_FileList.SelectedItem;
            string[] fileSplit = selectedDir.Split('.');
            if (fileSplit.Length < 3)
                return;
            string searchFile = "::" + nameSp + "::" + fileSplit[0] + "." + fileSplit[1] + "." + fileSplit[2];

            msg.add("namespaces", nameSp);
            msg.add("filename", fileSplit[0] + "." + fileSplit[1]);
            msg.add("searchFile", searchFile);
            msg.add("sender", "client");
            translater.postMessage(msg);
        }

        //----< Launch a file viewer >----------------
        private void launchFileWindow(string content, string filename)
        {
            statusBarText.Text = "Launched File Viewer";
            pop_up_window p = new pop_up_window();
            p.popup_content.Text = content;
            p.Show();
        }

        //----< Browse a file for checkin operation >----------------
        private void selectCheckinFile(object sender, RoutedEventArgs e)
        {
            // Create OpenFileDialog 
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            Nullable<bool> result = dlg.ShowDialog();
            // Get the selected file name and display in a TextBox 
            if (result == true)
            {
                // Open document 
                fileSelected = dlg.FileName;
                Brose_file_Path.Text = fileSelected;
            }
        }

        //----< referesh the files tab on checkin >----------------
        private void refreshWindow(string path)
        {
            // ../root/::namespace7::  
            statusBarText.Text = "File Browser Refreshed..";
            pathStack_checkin.Clear();
            Checkin_PathTextBlock.Text = path;
            RoutedEventArgs e = new RoutedEventArgs();
            loadDirectoriesCheckin(null,e);
            pathStack_checkin.Push("../"+path);
            Checkin_PathTextBlock.Text = path;
            Checkin_Console.Clear();
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("tab", "checkin");
            msg.add("path", pathStack_checkin.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< add dependency for file checkin >----------------
        private void addDepenency(object sender, RoutedEventArgs e)  //::namespace4::namespace5::file3.h.1
        {
            foreach (CheckBox s in Checkin_FileList.Items)
            {
                string directoryPath = Checkin_PathTextBlock.Text;
                string value = directoryPath.Split('/')[directoryPath.Split('/').Length - 1];

                if (s.IsChecked == true && !depen_list.Items.Contains("::"+value+"::"+s.Content))
                {   
                    depen_list.Items.Add("::"+value+"::"+s.Content);
                    statusBarText.Text = "Dependencies added";
                }
            }      
        }

        //----< add categories for file checkin >----------------
        private void addCategory(object sender, RoutedEventArgs e)
        {
            if (Category_Text.Text != "" && !Categories_list.Items.Contains(Category_Text.Text))
            {
                Categories_list.Items.Add(Category_Text.Text);
                statusBarText.Text = "Category Added";
            }
                
        }

        //----< remove categories for file checkin >----------------
        private void removeCategory(object sender, RoutedEventArgs e)
        {
            if ((string)Categories_list.SelectedValue != "")
            {
                Categories_list.Items.Remove(Categories_list.SelectedValue);
                statusBarText.Text = "Categories removed";
            }
              
        }


        private void performCheckin(object sender, RoutedEventArgs e)
        {
            foreach (CsMessage msg in fileList)
            {
                translater.postMessage(msg);
                msg.remove("command");
                msg.remove("file");
                msg.add("command", "checkin_complete");
                translater.postMessage(msg);
            }

            fileList.Clear();
        }

        //----< initaite a check in request to the server >----------------
        private void addFilesToList(object sender, RoutedEventArgs e)
        {
             string description = ""; string namespace_;
             string categories = "";string dependencies = "";string owner = "";
            statusSelected = status_box.Text;
            description = description_box.Text;
            statusBarText.Text = "Checking in file...";
            namespace_ = namespace_box.Text;
            string path = Brose_file_Path.Text;
            owner = owner_box.Text;
            if(path == "")
            {
                System.Windows.MessageBox.Show("Please Select a file", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                statusBarText.Text = "Error: Missing Manditory Field";
                return;
            }
            if (namespace_ == "")
            {
                System.Windows.MessageBox.Show("Please Enter a Namespace", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                statusBarText.Text = "Error: Missing Manditory Field";
                return;
            }
            if (owner == "")
            {
                System.Windows.MessageBox.Show("Please Enter Owner Name", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                statusBarText.Text = "Error: Missing Manditory Field";
                return;
            }
            foreach (string cat in Categories_list.Items)
                categories += cat + " ";
            foreach(string dep in depen_list.Items)
                dependencies += dep + " ";        
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            pathSelection(msg, path);
            msg.add("namespaces","::"+namespace_+"::"); msg.add("status",statusSelected);
            msg.add("description", description); msg.add("categories", categories.Trim());
            msg.add("dependencies", dependencies.Trim()); msg.add("file",path);
            msg.add("sender", "client");  msg.add("command", "checkin");
            msg.add("tab", "checkin");
            msg.add("owner", owner);
            fileList.Add(msg);
            file_list.Items.Add(msg.value("filename"));
            description_box.Clear(); namespace_box.Clear(); Category_Text.Clear();
            Categories_list.Items.Clear(); depen_list.Items.Clear();owner_box.Clear(); Brose_file_Path.Clear();
        }

        // for automation , utility function to fetch the right filename
        private void pathSelection(CsMessage msg , string path)
        {
             if (path.Contains("FileSystem") )          // to handle the test stubs, temp code
            {
                msg.add("filename", "FileSystem.h");
            }
            else if (path.Contains("IComm"))
            {
                msg.add("filename", "IComm.h");
            }
            else if (path.Contains("file4"))
            {
                msg.add("filename", "file4.cpp");
            }
            else if (path.Contains("file1"))
            {
                msg.add("filename", "file1.cpp");
            }
            else if (path.Contains("fileA"))
            {
                msg.add("filename", "fileA.h");
            }
            else if (path.Contains("fileB"))
            {
                msg.add("filename", "fileB.cpp");
            }
            else if(path.Contains("Message"))
            {
                msg.add("filename", "Message.h");
            }
            else if (path.Contains("Comm"))
            {
                msg.add("filename", "Comm.h");
            }
            else
            {
                msg.add("filename", path.Split('\\')[path.Split('\\').Length - 1]);
            }

        }

        // perform the execuution of the requested query
        private void executeQuery(object sender, RoutedEventArgs e)
        {
            string category = Category_Query.Text;
            string dependency = Dep_Query.Text;

            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "query_exec");
            msg.add("sender", "client");
            msg.add("tab", "browse");
            msg.add("queryType", "false");
            msg.add("category",category);
            msg.add("dependency", dependency);
            translater.postMessage(msg);
        }

        private void getNoParentsQuery(object sender, RoutedEventArgs e)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "query_exec");
            msg.add("tab", "browse");
            msg.add("queryType", "true");
            msg.add("sender", "client");
            translater.postMessage(msg);
        }

        // perform closure, persist db into database
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "close_system");
            translater.postMessage(msg);
        }

        // Automation stub for Client 2
        private async void automationStub(string port)
        {
            RoutedEventArgs e = new RoutedEventArgs();
            Console.WriteLine("Automation Unit Test For Client 2 Running at port  "+ port);
            Console.WriteLine("Demonstrating Checkout and Checkin respectively");
            Console.WriteLine("-----------------------------------------\n");
            Console.WriteLine("Demonstrating Connect requirement : ");
            Console.WriteLine("-----------------------------------------\n");
            await delay(2000);
            Machine_Port.Text = port;
            await delay(2500);
            connectServer(this, e);
            await delay(3000);
            Console.WriteLine("Connection Made with the server at Port 8080");
            automateCheckout2();
            await delay(14000);
            automateCheckin2();
            await delay(95000);
            automateCheckin3();

        }

        // Automation stub for Client 1
        private async void automationStub()
        {
            RoutedEventArgs e = new RoutedEventArgs();
            Console.WriteLine("Automation Unit Test For Client 1 Running at port 8082");
            Console.WriteLine("Demonstrating Checkout, Browse and View File, View Meta Data, Checkin Files respectively");
            Console.WriteLine("-----------------------------------------\n");
            Console.WriteLine("Demonstrating Connect requirement : ");
            Console.WriteLine("-----------------------------------------\n");
            connectServer(this, e);
            await delay(3000);
            Console.WriteLine("Connection Made with the server at Port 8080");
            automateCheckout();
            await delay(12000);
            automateBrowse();
            await delay(28000);
            automateViewMetadata();
            await delay(34000);
            automateCheckin();
 
        }

        //Delay the execution for async operations during automation
        //@param delay is in milliseconds
        async Task delay(int delay)
        {
            await Task.Delay(delay);
        }

        // Automation For Checkout Operations
        private async void automateCheckout()
        {
            loadDirectoriesCheckout(this, null);
            Console.WriteLine("\nDemonstrating Checkout requirement : ");
            Console.WriteLine("-----------------------------------------\n");
            tabControl.SelectedIndex = 2;
            await delay(2000);
            Console.WriteLine("\n On Checking out file4.cpp.1 , it will checkout all its transitive dependencies which includies , file4.h.1 , file1.cpp.1 , file1.h.1");
            Console.WriteLine("\n Note : file4.cpp.1 is transitively dependent on dependencies of file1.cpp.1 , which is file1.h.1 ");
            Check_DirList.SelectedIndex = Check_DirList.Items.IndexOf("namespace6");         // select directory
            await delay(2000);
            Check_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);
            Check_FileList.SelectedIndex = Check_FileList.Items.IndexOf("file4.cpp.1");  // select file to checkout
            await delay(2000);
            Check_FileList_MouseDoubleClick(this, null);
            await delay(2000);
            Console.WriteLine("All Files successfully checked out at location client_stub/");
        }

        // Automation For Browse Operations
        private async void automateBrowse()
        {
            loadDirectories(this,null);
            Console.WriteLine("\nDemonstrating Browse requirement And View File Requirement : ");
            Console.WriteLine("-----------------------------------------\n");
            tabControl.SelectedIndex = 3;
            await delay(2500);
            DirList.SelectedIndex = DirList.Items.IndexOf("namespace2");         // select directory
            await delay(2500);
            DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            FileList.SelectedIndex = FileList.Items.IndexOf("Comm.cpp.1");  // select file to Exract
            await delay(2500);
            FileList_MouseDoubleClick(this, null);
            await delay(2500);
            Category_Query.Text = "Cat2";
            await delay(2500);
            executeQuery(this,null);
            await delay(3000);
            Dep_Query.Text = "::namespace1::file1.h.1";
            await delay(2500);
            executeQuery(this, null);
            await delay(3000);
            getNoParentsQuery(this,null);
            await delay(3000);
            Console.WriteLine("File Comm.cpp.1 successfully extracted at location client_stub");
            Console.WriteLine("Browse Requirement Satisfied based on version and file name");
            Console.WriteLine("Query Based on Category and Dependency Satisfied");
            Console.WriteLine("Query to find all the files with no parents also Satisfied");
            Console.WriteLine("File Veiwer Launched to View Comm.h \n File Viewer Requirement Satisfied");
            await delay(1500);
        }

        // Automation For View Meta Data Operations
        private async void automateViewMetadata()
        {
            loadDirectoriesMeta(this,null);
            Console.WriteLine("\nDemonstrating View Metadata Of a file Requirement");
            Console.WriteLine("-----------------------------------------\n");
            tabControl.SelectedIndex = 4;
            await delay(3000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace6");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(3000);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("file4.cpp.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("..");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(3000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace1");         // select directory
            await delay(3000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(3000);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("file1.cpp.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(3000);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("file1.h.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            Console.WriteLine("Metadata Successfully Fetched for files file4.cpp.1 , file1.cpp.1 , file1.h.1 from the server");
            Console.WriteLine("View Meta Data Of a file Requirement Satisfied");
        }

        // Automation For Checkin Operations
        private async void automateCheckin_mult1()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\nDemonstrating Checkin a file Requirement, Message.h with open status ");
            Console.WriteLine("-----------------------------------------\n");
            tabControl.SelectedIndex = 1;
            await delay(1500);
            Brose_file_Path.Text = "../../../client_stage/Message.h";
            await delay(1500);
            status_box.Text = "Open";
            await delay(1500);
            description_box.Text = "Check in File Description for Message.h";
            await delay(1500);
            namespace_box.Text = "namespace7";
            await delay(1500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace1");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);
            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "file1.h.1")
                    c.IsChecked = true;
            }
            await delay(1000);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat1";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat2";
            await delay(1000);
            addCategory(this, null);
            await delay(1500);
            addFilesToList(this, null);
            Console.WriteLine("Message.h added to checkin with open status");
        }

        // Automation For Checkin Operations
        private async void automateCheckin_mult2()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\nDemonstrating Checkin a file Requirement, FileSystem.h with Closed status ");
            Console.WriteLine("-----------------------------------------\n");
            tabControl.SelectedIndex = 1;
            await delay(1500);
            Brose_file_Path.Text = "../../../client_stage/FileSystem.h";
            await delay(1000);
            status_box.Text = "Closed";
            await delay(1500);
            description_box.Text = "Check in File Description for FileSystem.h";
            await delay(1500);
            namespace_box.Text = "namespace8";
            await delay(1500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace3");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);
            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "file2.h.1")
                    c.IsChecked = true;
            }
            await delay(1500);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat5";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat6";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            addFilesToList(this, null);
            Console.WriteLine("FileSystem.h Successfully Checked into the server with Closed status");
        }

        // Automation For Checkin Operations
        private async void automateCheckin_mult3()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\nDemonstrating Checkin a file Requirement, FileSystem.h as Open On top of close status ");
            Console.WriteLine("-----------------------------------------\n");
            Console.WriteLine("\nNew Version is created FileSystem.h");
            tabControl.SelectedIndex = 1;
            await delay(1500);
            Brose_file_Path.Text = "../../../client_stage/FileSystem.h";
            await delay(1000);
            status_box.Text = "Closed";
            await delay(1500);
            description_box.Text = "Check in File Description for FileSystem.h";
            await delay(1500);
            namespace_box.Text = "namespace8";
            await delay(1500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace3");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);
            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "file2.h.1")
                    c.IsChecked = true;
            }
            await delay(1500);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat5";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat6";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            addFilesToList(this, null);
            Console.WriteLine("FileSystem.h Successfully Checked into the server with Closed status");
        }

        // Automation For Checkin Operations driver
        private async void automateCheckin()
        {
            automateCheckin_mult1();
            await delay(22000);
            automateCheckin_mult2();
            await delay(25000);
            performCheckin(this, null);
            await delay(3000);
            loadDirectoriesCheckin(this, null);
            await delay(2000);
            Console.WriteLine("Message.h Successfully Checked into the server with open status");
            Console.WriteLine("FileSystem.h Successfully Checked into the server with Closed status");
            Console.WriteLine("Checkin file Requirement Satisfied");
            checkMetadata1();
            await delay(32000);
            automateCheckin_mult3();
            await delay(25000);
            performCheckin(this, null);
            await delay(5000);
            loadDirectoriesCheckin(this, null);
            await delay(2000);
            automateCheckin_owner();
            await delay(23000);
            performCheckin(this, null);
            await delay(3000);
            tabControl.SelectedIndex = 0;
        }


        // Automation For Checkin Operations
        private async void automateCheckin_owner()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\nDemonstrating invalid owner name scenario, the checkin fails in this case ");
            Console.WriteLine("-----------------------------------------\n");
            tabControl.SelectedIndex = 1;
            await delay(1500);
            Brose_file_Path.Text = "../../../client_stage/Comm.h";
            await delay(1000);
            status_box.Text = "Closed";
            await delay(1500);
            description_box.Text = "Check in File Description for Comm.h";
            await delay(1500);
            namespace_box.Text = "namespace2";
            await delay(1500);
            owner_box.Text = "Xyz";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace3");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);
            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "file2.h.1")
                    c.IsChecked = true;
            }
            await delay(1500);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat5";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat6";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            addFilesToList(this, null);
            Console.WriteLine("Invalid owner , checkin fails");
        }

        // Automation For Meta Data View Operations
        private async void checkMetadata1()
        {
            Console.WriteLine("\nVerify Metadata Of Checked in Files");
            tabControl.SelectedIndex = 4;
            await delay(1500);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace7");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("Message.h.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("..");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace8");         // select directory
            await delay(2500);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("FileSystem.h.1");  // select file to Exract
            await delay(1500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(1500);
            Console.WriteLine("Metadata Successfully Fetched for files Message.h.1 , FileSystem.h.1 from the server");
        }

        // Automation For Checkout Operations
        private async void automateCheckout2()
        {
            Console.WriteLine("\nDemonstrating Checkout requirement : ");
            Console.WriteLine("-----------------------------------------\n");
            tabControl.SelectedIndex = 2;
            await delay(2000);
            Check_DirList.SelectedIndex = Check_DirList.Items.IndexOf("namespace3");         // select directory
            await delay(2000);
            Check_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);
            Check_FileList.SelectedIndex = Check_FileList.Items.IndexOf("file2.cpp.1");  // select file to checkout
            await delay(2000);
            Check_FileList_MouseDoubleClick(this, null);
            await delay(2000);
            Console.WriteLine("File file2.cpp.1 successfully checked out at location client_stub");
        }

        //Automation For Checkin Operations
        private async void automateCheckin_mult1_2()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\nDemonstrating Checkin a file Requirement, IComm.h with Closed status ");
            Console.WriteLine("-----------------------------------------\n");
            Console.WriteLine("\n IComm.h, goes into Closing state , as file4.cpp.1 is Open ");
            Console.WriteLine("\n On attepmt to Close file4.cpp.1, this file goes into Closing state , as file1.cpp.1 is open ");
            Console.WriteLine("\n On Closing file1.cpp.1 , both file4.cpp.1 and IComm.h get Closed ");
            tabControl.SelectedIndex = 1;
            await delay(1500);
            Brose_file_Path.Text = "../../../client_stage/IComm.h";
            await delay(1500);
            status_box.Text = "Closed";
            await delay(1500);
            description_box.Text = "Check in File Description for IComm.h";
            await delay(1500);
            namespace_box.Text = "namespace9";
            await delay(1500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace6");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);

            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "file4.cpp.1" || (string)c.Content == "file4.h.1")
                    c.IsChecked = true;
            }
            await delay(1000);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat8";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat9";
            await delay(1000);
            addCategory(this, null);
            await delay(1500);
            addFilesToList(this, null);
            Console.WriteLine("IComm.h added to checkin with Closing status");
        }

        // Automate the Checkin for Closing File
        private async void automateCheckin_mult2_2()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\n On attepmt to Close file4.cpp.1, this file goes into Closing state , as file1.cpp.1 is open ");
            Console.WriteLine("\n On Closing file1.cpp.1 , both file4.cpp.1 and IComm.h get Closed ");
            tabControl.SelectedIndex = 1;
            await delay(1500);
            Brose_file_Path.Text = "../../../client_stage/file4.cpp";
            await delay(1500);
            status_box.Text = "Closed";
            await delay(1500);
            description_box.Text = "Check in File Description for file4.cpp";
            await delay(1500);
            namespace_box.Text = "namespace6";
            await delay(1500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace1");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);

            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "file1.cpp.1")
                    c.IsChecked = true;
            }
            await delay(1000);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat2";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat4";
            await delay(1000);
            addCategory(this, null);
            await delay(1500);
            addFilesToList(this, null);
            Console.WriteLine("file4.cpp added to checkin with Closing status");
        }

        // Automate the Checkin for CLosing File
        private async void automateCheckin_mult3_2()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\n On Closing file1.cpp.1 , both file4.cpp.1 and IComm.h.1 get Closed ");
            tabControl.SelectedIndex = 1;
            await delay(1500);
            Brose_file_Path.Text = "../../../client_stage/file1.cpp";
            await delay(1500);
            status_box.Text = "Closed";
            await delay(1500);
            description_box.Text = "Check in File Description for file1.cpp";
            await delay(1500);
            namespace_box.Text = "namespace1";
            await delay(1500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace1");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);

            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "file1.h.1")
                    c.IsChecked = true;
            }
            await delay(1000);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat2";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat4";
            await delay(1000);
            addCategory(this, null);
            await delay(1500);
            addFilesToList(this, null);
            Console.WriteLine("file4.cpp added to checkin with Closing status");
        }
        // Automate view Metadata 
        private async void automateViewMetaData2_1()
        {
            Console.WriteLine("\nVerify Metadata Of Checked in File IComm.h");
            tabControl.SelectedIndex = 4;
            await delay(1500);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace9");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("IComm.h.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(1500);
            Console.WriteLine("Metadata Successfully Fetched for file IComm.h.1 from the server");

        }

        // Automate view Metadata 
        private async void automateViewMetaData2_2()
        {
            Console.WriteLine("\nVerify Metadata Of Checked in File file4.cpp.1");
            tabControl.SelectedIndex = 4;
            await delay(1500);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace6");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("file4.cpp.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(1500);
            Console.WriteLine("Metadata Successfully Fetched for file file4.cpp.1 from the server");

        }

        // Automate view Metadata 
        private async void automateViewMetaData2_3()
        {
            Console.WriteLine("\nVerify Metadata Of Checked in File");
            tabControl.SelectedIndex = 4;
            await delay(1500);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace1");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("file1.cpp.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(3000);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace6");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("file4.cpp.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(3000);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace9");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("IComm.h.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(3000);
            Console.WriteLine("file1.cpp1 , file4.cpp.1 and ICommh.1 , all successfully Transitively closed");

        }
        // Automation For Checkin Operations
        private async void automateCheckin2()
        {
            Console.WriteLine("\nDemonstrating Checkin a file Requirement With Closed status");
            Console.WriteLine("-----------------------------------------\n");

            tabControl.SelectedIndex = 1;
            loadDirectoriesCheckin(this,null);
            automateCheckin_mult1_2();
            await delay(26000);
            performCheckin(this, null);
            await delay(3000);
            automateViewMetaData2_1();
            await delay(20000);
            automateCheckin_mult2_2();
            await delay(26000);
            performCheckin(this, null);
            await delay(3000);
            automateViewMetaData2_2();
            await delay(20000);
            automateCheckin_mult3_2();
            await delay(26000);
            performCheckin(this, null);
            await delay(3000);
            automateViewMetaData2_3();
            await delay(20000);
            Console.WriteLine("\nPerforming Checkin Operation");
            performCheckin(this, null);
            await delay(5000);
        }

        // Automate the Checkin for CLosing File
        private async void automateCheckin_mult1_3()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\n Checkin fileA.h as Open , with no dependency");
            tabControl.SelectedIndex = 1;
            await delay(2500);
            Brose_file_Path.Text = "../../../client_stage/fileA.h";
            await delay(2500);
            status_box.Text = "Open";
            await delay(1500);
            description_box.Text = "Check in File Description for fileA.h";
            await delay(2500);
            namespace_box.Text = "namespace5";
            await delay(2500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Category_Text.Text = "Cat1";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat3";
            await delay(1000);
            addCategory(this, null);
            await delay(1500);
            addFilesToList(this, null);
            Console.WriteLine("fileA.h.1 added to checkin with Open status");
        }

        // Automate the Checkin for CLosing File
        private async void automateCheckin_mult2_3()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\n Checkin fileB.cpp as Closed , with fileA.h.1 as dependency");
            tabControl.SelectedIndex = 1;
            await delay(2500);
            Brose_file_Path.Text = "../../../client_stage/fileB.cpp";
            await delay(2500);
            status_box.Text = "Closed";
            await delay(2500);
            description_box.Text = "Check in File Description for fileB.pp";
            await delay(1500);
            namespace_box.Text = "namespace5";
            await delay(1500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace5");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);
            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "fileA.h.1")
                    c.IsChecked = true;
            }
            await delay(1500);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat1";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat3";
            await delay(1000);
            addCategory(this, null);
            await delay(1500);
            addFilesToList(this, null);
            Console.WriteLine("fileA.h.1 added to checkin with Open status");
        }

        // Automate the Checkin for CLosing File
        private async void automateCheckin_mult3_3()
        {
            loadDirectoriesCheckin(this, null);
            Console.WriteLine("\n Checkin fileA.h as Open , with no dependency");
            tabControl.SelectedIndex = 1;
            await delay(2500);
            Brose_file_Path.Text = "../../../client_stage/fileA.h";
            await delay(2500);
            status_box.Text = "Closed";
            await delay(1500);
            description_box.Text = "Check in File Description for fileA.h";
            await delay(2500);
            namespace_box.Text = "namespace5";
            await delay(2500);
            owner_box.Text = "Himanshu";
            await delay(1500);
            Checkin_DirList.SelectedIndex = Checkin_DirList.Items.IndexOf("namespace5");         // select directory
            await delay(2000);
            Checkin_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2000);
            foreach (CheckBox c in Checkin_FileList.Items)
            {
                if ((string)c.Content == "fileB.cpp.1")
                    c.IsChecked = true;
            }
            await delay(1500);
            addDepenency(this, null);
            await delay(1500);
            Category_Text.Text = "Cat1";
            await delay(1000);
            addCategory(this, null);
            await delay(1000);
            Category_Text.Text = "Cat3";
            await delay(1000);
            addCategory(this, null);
            await delay(1500);
            addFilesToList(this, null);
            Console.WriteLine("fileA.h.1 added to checkin with Closed status");
        }
        // Automate view Metadata 
        private async void automateViewMetaData3_1()
        {
            Console.WriteLine("\nVerify Metadata Of Checked in File fileB.cpp.1");
            tabControl.SelectedIndex = 4;
            await delay(1500);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace5");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("fileB.cpp.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(1500);
            Console.WriteLine("Metadata Successfully Fetched for file fileB.cpp.1 from the server");
        }

        // Automate view Metadata 
        private async void automateViewMetaData3_2()
        {
            Console.WriteLine("\nVerify Metadata Of Checked in File fileA.h.1 and fileB.cpp.1");
            tabControl.SelectedIndex = 4;
            await delay(1500);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace5");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("fileA.h.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(1500);
            loadDirectoriesMeta(this, null);
            await delay(2000);
            Meta_DirList.SelectedIndex = Meta_DirList.Items.IndexOf("namespace5");         // select directory
            await delay(2000);
            Meta_DirList_MouseDoubleClick(this, null);     // fetch files of that directory
            await delay(2500);
            Meta_FileList.SelectedIndex = Meta_FileList.Items.IndexOf("fileB.cpp.1");  // select file to Exract
            await delay(2500);
            Meta_FileList_MouseDoubleClick(this, null);
            await delay(1500);
            Console.WriteLine("Metadata Successfully Fetched for file fileB.cpp.1 from the server");

        }
        // Automation For Checkin Operations
        private async void automateCheckin3()
        {
            await delay(75000);
            Console.WriteLine("\nDemonstrating Circular Dependency resolution");
            Console.WriteLine("-----------------------------------------\n");
            Console.WriteLine("\n1.Checkin fileA.h as open \n2.Checkin fileB.cpp as Closed file , with fileA.cpp as dependency ");
            Console.WriteLine("3.fileB.cpp.1 goes into Closing state \n4.Close fileA.h.1 with fileB.cpp.1 as a circular dependency");
            Console.WriteLine("5.fileA.h.1 and fileB.cpp.1 both get closed");
            tabControl.SelectedIndex = 1;
            loadDirectoriesCheckin(this, null);
            automateCheckin_mult1_3();
            await delay(28000);
            performCheckin(this, null);
            await delay(3500);
            automateCheckin_mult2_3();
            await delay(26000);
            performCheckin(this, null);
            await delay(3000);
            automateViewMetaData3_1();
            await delay(17000);
            automateCheckin_mult3_3();
            await delay(26000);
            performCheckin(this, null);
            await delay(3000);
            automateViewMetaData3_2();
            await delay(20000);
            performCheckin(this, null);
            await delay(5000);
            tabControl.SelectedIndex = 0;
        }


        // Automation For Disconnect Operations
        private async void automateDisconnect()
        {
            RoutedEventArgs e = new RoutedEventArgs();
            Console.WriteLine("\nDemonstrating Client Disconnecting from Server");
            Console.WriteLine("-----------------------------------------\n");
            tabControl.SelectedIndex = 0;
            await delay(2000);
            disconnectServer(this,e);
            Console.WriteLine("Client Disconnected from server");
        }

       
    }
}
