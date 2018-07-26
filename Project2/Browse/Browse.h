#pragma once
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../FileMgr/FileMgr.h"
#include "../NoSQLDB/PayLoad/PayLoad.h"
#include "../Process/Process/Process.h"
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////
// Browse.h													         //
// Operations Performed: Allows  to view the requested file        //
// Also view its dependencies										 //	
// ver 1.0                                                           //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018    //
///////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package provides
* - This package provides mechanism to perform Browse the files in the repo
	- browseFiles () is the only public function which is exposed which performs following
		- Launch the requested file on notepad
		- Display the records information on console
		- Provides with details of transitive dependencies
		- Method is provided with the repo db access along with the file key and transitive
		  dependencylist to be Browsed

* Required Files:
* ---------------
- NoSqlDb.h , NoSqlDb.cpp
- FileMgr.h , FileMgr.cpp
- Process.h . Process.cpp
*
* Maintenance History:
* --------------------
* ver 1.0 : 08 March 2018
*/

using namespace NoSqlDb;
using namespace std;

class Browse
{
public:
	Browse();
	~Browse();

	template<typename T> static void browseFiles(DbCore<T>& db, vector<std::string> keys);

private:
    static void startProcess(const std::string& filepath);
};


template<typename T> void Browse::browseFiles(DbCore<T>& db, vector<std::string> keys)
{
	startProcess(db[keys[0]].payLoad().value());
	showHeader();
	showRecord(keys[0], db[keys[0]]);
	std::cout << "\n\n Transitive Dependency list of the file include following files";
	for (size_t i = 1; i < keys.size(); i++) {
		std::cout << "\n File : " << keys[i];
	}
}


inline void Browse::startProcess(const std::string& filepath)
{
	
	Process p;
	p.title("Browse File");
	std::string appPath = "C:/Windows/System32/notepad.exe";//"c:/windows/system32/notepad.exe";
	p.application(appPath);

	std::string cmdLine = "/A ";
	cmdLine.append(filepath);
	p.commandLine(cmdLine);
	p.create();

	CBP callback = []() { std::cout << "\n  --- child process exited with this message ---"; };
	p.setCallBackProcessing(callback);
	p.registerCallback();

	std::cout.flush();
}
