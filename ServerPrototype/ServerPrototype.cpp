/////////////////////////////////////////////////////////////////////////
// ServerPrototype.cpp - Console App that processes incoming messages  //
// ver 2.0                                                             //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018			//
// Author : Himanshu Chhabra											//
/////////////////////////////////////////////////////////////////////////

#include "ServerPrototype.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include <chrono>
#include <sstream>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using namespace NoSqlDb;
using Msg = MsgPassingCommunication::Message;

DbCore<PayLoad> db;
//----< getfiles  of a directory>----------------
Files Server::getFiles(const Repository::SearchPath& path)
{
  return Directory::getFiles(path);
}
//----< getdirectores  of a directory>----------------
Dirs Server::getDirs(const Repository::SearchPath& path)
{
  return Directory::getDirectories(path);
}

//----< show contents of a file >----------------
template<typename T>
void show(const T& t, const std::string& msg)
{
  std::cout << "\n  " << msg.c_str();
  for (auto item : t)
  {
    std::cout << "\n    " << item.c_str();
  }
}

//----< process echo request >----------------
std::function<Msg(Msg)> echo = [](Msg msg) {
  Msg reply = msg;
  reply.to(msg.from());
  reply.from(msg.to());
  return reply;

};

//----< process connection request >----------------
std::function<Msg(Msg)> connectionRequest = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("connectionReq");
	reply.attribute("content", "Connected to server");
	// Read from XML , Instantiate the Database
	std::string readXml, inputXml;
	std::ifstream inputFile("../readXML.xml");
	Persist<PayLoad> p(db);

	while (inputFile >> readXml) {
		inputXml.append(readXml);
	}
	inputFile.close();
	bool read = p.fromXml(inputXml, true);

	if (read) {
		std::cout << "\n\n DB Successfully populated from readXML.xml \n";
		std::cout << "\n\n Current Repository DB \n";
		std::cout << "--------------------------------\n";
		//showDb(db);
		std::cout << "\n\n Current Repo DB Payload Status\n";
		std::cout << "--------------------------------\n";
		//PayLoad::showDb(db);
	}
	return reply;
};
// Extract the file names form the key
std::string listExtractedFiles(std::vector<std::string> result) {
	std::string resMsg;

	for (size_t i = 0; i < result.size(); i++) {
		std::string val = result[i].substr(result[i].find_last_of("::") + 1);
		val = val.substr(0,val.find_last_of("."));
		if (i != result.size() - 1) {
			resMsg += val + " ";
		}
		else {
			resMsg += val;
		}
	}

	return resMsg;
}

//----< process checkout request >----------------
std::function<Msg(Msg)> checkOutFiles = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	RepositoryCore<PayLoad> repo(db);
	stringstream version(msg.value("version"));
	size_t ver;
	version >> ver;
	std::vector<std::string> result;
	std::string tab = msg.value("tab");
	if (tab == "checkout") {
		result = repo.checkoutFile(msg.value("searchFile"), ver);
	}
	else {
		result = repo.checkoutFile(msg.value("searchFile"), ver);
	}
	std::string allFiles = listExtractedFiles(result);
	reply.command("getFiles_response");
	reply.attribute("sender", "server");
	reply.attribute("content", "Checkout Process started");
	if (tab == "checkout") {
		reply.attribute("allFiles", allFiles);
		reply.attribute("key", result[0]);
	}
	else {
		std::string val = result[0].substr(result[0].find_last_of("::") + 1);
		val = val.substr(0, val.find_last_of("."));
		reply.attribute("allFiles", val);
		reply.attribute("key" , result[0]);
	}
	return reply;
};

//----< process getFilesOperation request >----------------
std::function<Msg(Msg)> getFilesFromServ = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	std::string filenames = msg.value("filename");
	reply.attribute("filenames", filenames);
	reply.attribute("file", filenames);
	reply.command("extractFiles_response");
	reply.attribute("sender", "server");
	reply.attribute("content", " file checkout in progress..");
	if (reply.value("tab") == "browse") {
		std::string key = reply.value("key");
		DbElement<PayLoad> elem = db[key];
		reply.attribute("filenames", filenames);
		reply.attribute("description", elem.descrip());
		std::vector<std::string> categories = elem.payLoad().categories();
		std::string catList;
		for (size_t i = 0; i < categories.size(); i++) {
			catList += categories[i] + " ";
		}
		reply.attribute("Categories", catList);
		std::vector<std::string> children = elem.children();
		std::string childrenList;
		for (size_t i = 0; i < children.size(); i++) {
			childrenList += children[i] + " ";
		}
		reply.attribute("Children", childrenList);
		reply.attribute("Owner", elem.name());
		reply.attribute("DateTime", elem.dateTime());
		std::string status;
		if (elem.payLoad().getCheckinStatus() == closed) {
			status = "Closed";
		}
		else if (elem.payLoad().getCheckinStatus() == open) {
			status = "Open";
		}
		else {
			status = "Closing";
		}
		reply.attribute("Status", status);
	}
	return reply;
};

//----< process persistDb request >----------------
std::function<Msg(Msg)> persistDb = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("close_system_response");
	reply.attribute("sender", "server");
	Persist<PayLoad> p(db);
	std::string xml = p.toXml();
	std::ofstream outputFile("../readXML.xml");
	outputFile << xml;
	return reply;
};



//----< process meta data request >----------------
std::function<Msg(Msg)> fetchMetaData = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("fetchMetaData_response");
	reply.attribute("sender", "server");
	std::string namespaces = msg.value("namespaces");
	std::string filename = msg.value("filename");
	std::string searchFile = msg.value("searchFile");
	DbElement<PayLoad> elem = db[searchFile];
	reply.attribute("filenames", filename);
	reply.attribute("description", elem.descrip());
	std::vector<std::string> categories = elem.payLoad().categories();
	std::string catList;
	for (size_t i = 0; i < categories.size(); i++) {
		catList += categories[i] + " ";
	}
	reply.attribute("Categories", catList);
	std::vector<std::string> children = elem.children();
	std::string childrenList;
	for (size_t i = 0; i < children.size(); i++) {
		childrenList += children[i] + " ";
	}
	reply.attribute("Children", childrenList);
	reply.attribute("Owner",elem.name());
	reply.attribute("DateTime", elem.dateTime());
	std::string status;
	if (elem.payLoad().getCheckinStatus() == closed) {
		status = "Closed";
	}
	else if(elem.payLoad().getCheckinStatus() == open){
		status = "Open";
	}
	else {
		status = "Closing";
	}
	reply.attribute("Status", status);
	reply.attribute("content", " MetaData for File: " + filename);
	return reply;
};

//----< process checkin request >----------------
std::function<Msg(Msg)> checkInFile = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.remove("file");
	reply.attribute("command", "checkin_progress");
	reply.attribute("content", " File Checkin in progress");
	std::cout << "\nhere";
	return reply;
};

//----< process checkin request >----------------
std::function<Msg(Msg)> checkInFileComplete = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());reply.from(msg.to());
	reply.attribute("command", "checkin_response");reply.attribute("content", " File Checkin Completed");reply.attribute("sender", "server");
	std::vector<DbElement<PayLoad>> elementList;
	DbElement<PayLoad> elem;
	std::string namespace_ = reply.value("namespaces");std::string status = reply.value("status");
	std::string description = reply.value("description");std::string filename = reply.value("filename");
	if (reply.value("owner") != db["root"].name()) {
		reply.attribute("content", "Checkin Failed User Not Authorized to perform desired operation");reply.attribute("stat", "fail");
		return reply;
	}
	reply.attribute("stat", "pass");
	elem.name(reply.value("owner"));elem.descrip(description);elem.dateTime(DateTime().now());elem.payLoad().value("../Storage/" + filename);   
	elem.payLoad().setNamespace(namespace_);
	if (status == "Closed") {
		elem.payLoad().setCheckinStatus(closed);
	}
	else {
		elem.payLoad().setCheckinStatus(open);
	}
	std::string categories = reply.value("categories");
	istringstream iss(categories);
	do {
		std::string cat;
		iss >> cat;
		if (cat == "")
			break;
		elem.payLoad().categories().push_back(cat);
	} while (iss);
	std::string dependencies = reply.value("dependencies");
	istringstream isd(dependencies);
	do {
		std::string dep;
		isd >> dep;
		if (dep == "")
			break;
		elem.children().push_back(dep);
	} while (isd);
	elementList.push_back(elem);
	RepositoryCore<PayLoad> r(db);
	db = r.submitFiles(elementList);
	Persist<PayLoad> p(db);
	std::string xml = p.toXml();
	std::ofstream outputFile("../readXML.xml");
	outputFile << xml;
	return reply;
};


//----< process query Exec request >----------------
std::function<Msg(Msg)> queryExecute = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.attribute("command", "query_exec_response");
	reply.attribute("content", "Eexecute File Result");
	std::string category = msg.value("category");
	std::string dependency = msg.value("dependency");
	RepositoryCore<PayLoad> repo(db);
	std::vector<std::string> result;
	if (msg.value("queryType") == "false") {
	    result = repo.execQuery(category, dependency, false);
	}
	else {
		result = repo.execQuery(category, dependency, true);
	}
	
	if (result.size() > 0) {
		std::string fileList = listExtractedFiles(result);
		std::string val = result[0].substr(result[0].find_last_of("::") + 1);
		//val = val.substr(0, val.find_last_of("."));
		reply.attribute("fileList", fileList);
		reply.attribute("filename", val);
		reply.attribute("key", result[0]);
		reply.attribute("status", "success");
	}
	else {
		reply.attribute("status", "failure");
	}
	

	return reply;
};



//----< process get files request >----------------
std::function<Msg(Msg)> getFiles = [](Msg msg) {
  Msg reply;
  reply.to(msg.from());
  reply.from(msg.to());
  reply.command("getFiles");
  reply.attribute("tab",msg.value("tab"));
  std::string path = msg.value("path");
  if (path != "")
  {
    std::string searchPath = storageRoot;
    if (path != ".")
      searchPath = searchPath + "\\" + path;
    Files files = Server::getFiles(searchPath);
    size_t count = 0;
    for (auto item : files)
    {
      std::string countStr = Utilities::Converter<size_t>::toString(++count);
      reply.attribute("file" + countStr, item);
    }
  }
  else
  {
    std::cout << "\n  getFiles message did not define a path attribute";
  }
  return reply;
};

//----< process get dirs request >----------------
std::function<Msg(Msg)> getDirs = [](Msg msg) {
  Msg reply;
  reply.to(msg.from());
  reply.from(msg.to());
  reply.command("getDirs");
  reply.attribute("tab", msg.value("tab"));
  std::string path = msg.value("path");
  if (path != "")
  {
    std::string searchPath = storageRoot; // use the root
    if (path != ".")
      searchPath = searchPath + "\\" + path;
    Files dirs = Server::getDirs(searchPath);
    size_t count = 0;
    for (auto item : dirs)
    {
      if (item != ".." && item != ".")
      {
        std::string countStr = Utilities::Converter<size_t>::toString(++count);
        reply.attribute("dir" + countStr, item);
      }
    }
  }
  else
  {
    std::cout << "\n  getDirs message did not define a path attribute";
  }
  return reply;
};

int main()
{
  std::cout << "\n  Testing Server Prototype";
  std::cout << "\n ==========================";
  std::cout << "\n";Server server(serverEndPoint, "ServerPrototype");
  server.start();
  std::cout << "\n  testing getFiles and getDirs methods";
  std::cout << "\n --------------------------------------";
  Files files = server.getFiles();
  show(files, "Files:");
  Dirs dirs = server.getDirs();
  show(dirs, "Dirs:");
  std::cout << "\n";
  std::cout << "\n  testing message processing";
  std::cout << "\n ----------------------------";
  server.addMsgProc("echo", echo);
  server.addMsgProc("getFiles", getFiles);
  server.addMsgProc("getDirs", getDirs);
  server.addMsgProc("serverQuit", echo);
  server.addMsgProc("connectionRequest", connectionRequest);
  server.addMsgProc("extractFiles", checkOutFiles);
  server.addMsgProc("fetchMetaData", fetchMetaData);
  server.addMsgProc("checkin", checkInFile); 
  server.addMsgProc("checkin_complete", checkInFileComplete);
  server.addMsgProc("getFilesFromServ", getFilesFromServ);
  server.addMsgProc("close_system", persistDb);
  server.addMsgProc("query_exec", queryExecute);
  server.processMessages();
  Msg msg(serverEndPoint, serverEndPoint);  // send to self
  msg.name("msgToSelf");
  msg.command("echo");
  msg.attribute("verbose", "show me");
  server.postMessage(msg);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  msg.command("getFiles");
  msg.remove("verbose");
  msg.attributes()["path"] = storageRoot;
  server.postMessage(msg);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  msg.command("getDirs");
  msg.attributes()["path"] = storageRoot;
  server.postMessage(msg);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  std::cout << "\n  press enter to exit";
  std::cin.get();
  std::cout << "\n";
  msg.command("serverQuit");
  server.postMessage(msg);
  server.stop();
  return 0;
}

