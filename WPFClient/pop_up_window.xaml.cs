/////////////////////////////////////////////////////////////////////////
//pop_up_window.xaml.cs - Launches a Pop up window to view the file    //
// ver 1.0                                                             //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018      //
/////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
*  - This package provides allows the user to view the selected file during browse operation
*  - Launches the selcted file on a separatw window to view the full file.
*  
* Required Files:
* ---------------
* pop_up_window.xaml, pop_up_window.xaml.cs
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
using System.Windows.Shapes;

namespace WPFClient
{
    /// <summary>
    /// Interaction logic for pop_up_window.xaml
    /// </summary>
    public partial class pop_up_window : Window
    {
        public pop_up_window()
        {
            InitializeComponent();
        }
    }
}
