#pragma once
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../FileMgr/FileMgr.h"
#include "../NoSQLDB/PayLoad/PayLoad.h"
//#include "../CodeRepo/RepositoryCore.h"
#include <string>
#include <vector>
#include <algorithm>

/////////////////////////////////////////////////////////////////////
// Checkin.h 										               //
// Operations Performed :copy files from User space to the repo	   //
// ver 1.0                                                         //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018  //
/////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package provides
* - This package provides mechanism to perform submition of files into the repo
  - submit () is the only public function which is exposed which performs following
		- Copy the file from the user space to repo
		- Make sure files are checkdin with correct version
		- Maintain the checkin status of the files.
		- Maintain information about dependencies.
		- Method is provided with the repo db access along with the file key to be checkedin
  - onFileCloseRequest() - Resolves dependencies before closing the file, maintains dependency dependentOn map 
	which holds info about which files are waiting for which other files to be closed

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

class Checkin
{
public:
	Checkin();
	~Checkin();
	template<typename T> static void submit(DbCore<T>& db, std::string key, DbElement<T>& dbelement,std::size_t version, std::string des);
	template<typename T> static void onFileCloseRequest(std::string key, DbCore<T>& db);
	static std::unordered_map<Key, vector<std::string>> dependentOn;
private:
	//template<typename T> static void onFileCloseRequest(std::string key, DbCore<T>& db);
	template<typename T> static void perfromClosure(Key key , DbCore<T>& db);
	//static std::unordered_map<Key, vector<std::string>> dependentOn;				// vector of parents
	static std::unordered_map<Key, vector<std::string>> parentOf;					// vector of children
	template<typename T> static void removeOpenFiles(vector<std::string>& keyList , DbCore<T>& db, std::string key);
};



/*
Perofrms the checkin operation ,Fetch the status of the existiing version
check the status if open then override , if close then create new, if its 0, then checkin simply
*/
template<typename T> void Checkin::submit(DbCore<T>& db, std::string key, DbElement<T>& dbelement, std::size_t version, std::string des)
{
	bool copied = false;
	if (version == 0) {
		copied = FileSystem::File::copy(dbelement.payLoad().value(), des.append(".").append(std::to_string(++version)), false);
		if (copied) {
			dbelement.payLoad().value(des);
			db[key.append(".").append(std::to_string(version))] = dbelement;
		}
	}
	else {
		state st = db[key.append(".").append(std::to_string(version))].payLoad().getCheckinStatus();
		switch (st)
		{
		case NoSqlDb::open:
			copied = FileSystem::File::copy(dbelement.payLoad().value(), des.append(".").append(std::to_string(version)), false);
			if (copied) {
				dbelement.payLoad().value(des);
				db[key] = dbelement;
			}
			break;
		case NoSqlDb::closed:
			copied = FileSystem::File::copy(dbelement.payLoad().value(), des.append(".").append(std::to_string(++version)), false);
			if (copied) {
				dbelement.payLoad().value(des);
				key = key.substr(0, key.find_last_of(".") + 1).append(std::to_string(version));
				db[key] = dbelement;
			}
			break;
		case NoSqlDb::closing:
			copied = FileSystem::File::copy(dbelement.payLoad().value(), des.append(".").append(std::to_string(++version)), false);
			if (copied) {
				dbelement.payLoad().value(des);
				key = key.substr(0, key.find_last_of(".") + 1).append(std::to_string(version));
				db[key] = dbelement;
			}
			break;
		default:
			std::cout << "Invalid Checkin State \n\n";
			break;
		}
	}
	if (copied && db[key].payLoad().getCheckinStatus() == closed)
		onFileCloseRequest(key,db);
}

// Determines if the file can be closed, if not changes the status of the file to closing (pending)
// Maintains a list of all dependencies in case of file status was changed to closing (pending)
template<typename T> void Checkin::onFileCloseRequest(std::string key,   DbCore<T>& db) {
	vector<std::string> keyList = RepositoryCore<T>::extractFiles(key,db);		// fetch the dependencies List
	keyList.erase(keyList.begin());		// removing self
	if (keyList.empty()) {
		perfromClosure(key , db);
	}
	else {
		removeOpenFiles(keyList,db,key);
		if (keyList.empty()) {
			perfromClosure(key , db);
		}
		else {//call a method and check if any member of keyList is dependent on key itself, if yes remove it from keyList, check if after this operation keyList.empty() ,if yes call closure else continue
			std::cout << "\n\n File " << key << " checkin status changed to closing(1)";
			std::cout << "\n Waiting for following files to be closed :";
			dependentOn[key] = keyList;
			for (size_t i = 0; i < keyList.size(); i++) {
					parentOf[keyList[i]].push_back(key);
					std::cout << " " << keyList[i];
			}
			db[key].payLoad().setCheckinStatus(closing);
		}
	}
}

// Performs the closing operations on a file recursively by notifying all the files waiting for file being closed
template<typename T> void Checkin::perfromClosure(Key key , DbCore<T>& db) {
	
	std::unordered_map<std::string, vector<std::string>>::iterator valFound = parentOf.find(key);
	vector<std::string> keyList;

	if (valFound != parentOf.end())
		keyList = valFound->second;

	//vector<std::string> keyList = parentOf[key];								// fetch the children list
	if (!keyList.empty()) {
		for (size_t i = 0; i < keyList.size(); i++) {
			vector<std::string> list = dependentOn[keyList[i]];					// remove myself from childs wait list

			std::vector<std::string>::iterator position = std::find(list.begin(), list.end(), key);
			if (position != list.end()) {							 // == myVector.end() means the element was not found
				list.erase(position);
				dependentOn[keyList[i]] = list;
			}

			if (list.empty())										// if i was the last parent the child was waiting to get closed then close it before me.
				perfromClosure(keyList[i] , db);
		}
	}
	
	db[key].payLoad().setCheckinStatus(closed);
	// remove the key from parents list to avoid memory leak
}

template<typename T> void Checkin::removeOpenFiles(vector<std::string>& keyList , DbCore<T>& db, std::string key) {
	for (size_t i = 0; i < keyList.size(); i++) {
		if (db[keyList[i]].payLoad().getCheckinStatus() != open) {		// keyList will have only those which are open 
			if (db[keyList[i]].payLoad().getCheckinStatus() == closing) {
				if (parentOf.find(key) != parentOf.end()) {
					vector<std::string> list = parentOf[key];
					std::vector<std::string>::iterator position = std::find(list.begin(), list.end(), keyList[i]);
					if (position != list.end()) {
						keyList.erase(keyList.begin() + i);
						i--;
					}
				}
			}
			else {
				keyList.erase(keyList.begin() + i);
				i--;
			}
		}
	}
}