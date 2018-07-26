#pragma once
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../FileMgr/FileMgr.h"
//#include "../NoSQLDB/PayLoad/PayLoad.h"
#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////
// Checkout.h 										                //
// Operations Performed :copy files from Repo to the User Space	   //
// ver 1.0                                                         //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018  //
/////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package provides
* - This package provides mechanism to perform extraction of files into the repo
- copyToStage () is the only public function which is exposed which performs following
	- Copy the file from the repo to user space
	- Copy all the Transitive depencies along with the requested file.
	- Method is provided with the repo db access along with the file key and transitive
	  dependencylist to be checkedout

* Required Files:
* ---------------
- NoSqlDb.h , NoSqlDb.cpp
- FileMgr.h , FileMgr.cpp

*
* Maintenance History:
* --------------------
* ver 1.0 : 08 March 2018
*/

using namespace NoSqlDb;
using namespace std;

class Checkout
{
public:
	Checkout();
	~Checkout();

	template<typename T> static bool copyToStage(DbCore<T> db, vector<std::string> resultKeys, std::string des);

private:

};


/*
Copies the files along with all its transitive dependcies to the user space (stage area)
*/
template<typename T>  bool Checkout::copyToStage(DbCore<T> db, vector<std::string> resultKeys, std::string des) {
	std::cout << " \n\n List of Files Checkout along with Transitive Dependencies";
	for (size_t i = 0; i < resultKeys.size(); i++) {
		if (db.contains(resultKeys[i])) {
			DbElement<T> dbelement = db[resultKeys[i]];
			std::string val = resultKeys[i].substr(resultKeys[i].find_last_of("::") + 1);
			std::cout << " \n File : " << resultKeys[i];
			bool copied = FileSystem::File::copy(dbelement.payLoad().value(), des.append("/").append(val.substr(0,val.find_last_of("."))), false);
			des = des.substr(0,des.find_last_of("/"));
		}
	}
	
	return true;
}
