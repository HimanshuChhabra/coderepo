#pragma once
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../FileMgr/FileMgr.h"
#include "../Version/Version.h"
#include "../Checkin/Checkin.h"
#include "../Checkout/Checkout.h"
#include "../Browse/Browse.h"
#include <string>
#include <vector>
#include <regex>
#include <sstream>


/////////////////////////////////////////////////////////////////////
// RepositoryCore.h - provides operations and utilities for		   //
// checkin, checkout, browse operations							   //
// Operations Performed :Invokes the Test stubs					   //
// ver 1.0                                                         //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018  //
/////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package provides
* - This package is responsible for forwarding the checkin, checkout, browse requests to designated classes
- Policy - the namepsace of the file requested to checkin is used to create the directory structure
			eg: name1::name2::file1.cpp => ../root/name1/name2/file1.cpp
		 - The root of the file is set to ../root 
  - Following public functions are exposed.
  - submitFiles() - accepts a batch or vector of files to be checked in
	- iteratively the files are forwareded to Checkin class for checkin one at a time.
  - checkoutFile() - accpets the filename along with version number of the file to be checkedout
	- if no version is provided latest version of the file will be checkdout
  -  browseRepo() - accpets the filename along with version number of the file to be browsed
	- if no version is provided latest version of the file will be browsed
  - getDB() - returns the repository data base
  - extractFiles() - accepts a filename, and returns a vector of transitive dependencies keys
  - execQuery() - allows the user to query based on category and dependencies

* Required Files:
* ---------------
- NoSqlDb.h , NoSqlDb.cpp
- FileMgr.h , FileMgr.cpp
- Version.h , Version.cpp
- Checkin.h , Checkin.cpp
- Checkout.h , Checkout.cpp
- Browse.h , Browse.cpp

*
* Maintenance History:
* --------------------
* ver 1.0 : 08 March 2018
*/

using namespace NoSqlDb;
using namespace FileManager;
using namespace std;

template<typename T>
class RepositoryCore
{
public:

	RepositoryCore(DbCore<T>& db) { repoDB = db; initMaps();
	}

	DbCore<T>& submitFiles(std::vector<DbElement<T>> checkInElements);
	vector<std::string> browseRepo(std::string filename, std::size_t version);
	DbCore<T>& getDB() { return repoDB; }
	std::vector<std::string> checkoutFile(std::string filename, std::size_t version);
	std::vector<std::string> execQuery(std::string category, std::string dependency , bool noParent);
	static std::vector<std::string> extractFiles(std::string key, DbCore<T>& db);
private:
	DbCore<T>  repoDB;
	void createDirectories(const std::string & path, std::string & remainder, std::size_t offset);
	size_t directoryExists(const std::string & path);
	std::string createPathFromNameSpace(const std::string& namespaces);
	std::size_t getFileVersion(std::string path);
	std::string createDirectoryStructure(std::string namespaces);
	std::vector<std::string> getKeys(std::string filename);
	std::string getKeyByFilename(std::string filename, std::size_t version);
	static std::vector<std::string> copyValues(std::vector<std::string> src, std::vector<std::string> des);
	static bool contains(std::vector<std::string> src, std::string key);
	bool authenticateUser(std::string username);
	void initMaps();
};

/*
1. for every db element, get the payload , get the status check if its closing , if yes
2. add the to depenedncy map , inverse parent map
*/
template<typename T>
inline void RepositoryCore<T>::initMaps() {

	DbCore<PayLoad> db = getDB();
	std::vector<std::string> keys = db.keys();
	for (size_t i = 0; i < keys.size(); i++) {
		DbElement<PayLoad> elem = db[keys[i]];
		if (elem.payLoad().getCheckinStatus() == closing) {
			if(Checkin::dependentOn.find(keys[i]) == Checkin::dependentOn.end())
				Checkin::onFileCloseRequest(keys[i],db);
		}
	}
	
}

/*
fetch the latest version of the file given the path of the file
*/
template<typename T>
inline std::size_t RepositoryCore<T>::getFileVersion(std::string path) {
	Query<PayLoad> q1(getDB());
	path = path.append(".*");
	// need  to accept key and turn it into regex
	std::vector<std::string> keys = q1.selectKeys(path).keys();  //"::namespace1::namespace2::demo1.cpp.*"
	Version v;
	return v.getFileVersion(keys);
}

/*
Creates the directory structure of the file, given the path of the file
*/
template<typename T>
inline void RepositoryCore<T>::createDirectories(const std::string& path, std::string& remainder, std::size_t offset) {
	std::string val = path;
	while (remainder != "") {
		std::size_t remainOffset = remainder.find_first_of("/") + 1;
		val = val.substr(0, offset + 1).append(remainder.substr(0, remainOffset));        // substring 0 , 5 is   0,1,2,3,4
		FileSystem::Directory::create(val);
		remainder = remainder.substr(remainOffset);
		offset += remainOffset;
	}
}

/*
returns the index of the string @arg path uptil where the directory structure exists in the file system
*/
template<typename T>
inline size_t RepositoryCore<T>::directoryExists(const std::string& path) {
	bool result = FileSystem::Directory::exists(path);
	std::size_t found = path.size();

	if (!result) {
		std::size_t found = path.find_last_of("/");
		return directoryExists(path.substr(0, found));
	}
	else {
		return found;
	}

}

/*
construct a diretcory path of the file form the namespace
*/
template<typename T>
inline std::string RepositoryCore<T>::createPathFromNameSpace(const std::string& namespaces) {
	std::string rootPath =  getDB()["root"].payLoad().value();
	std::string namesp = namespaces;//namespaces.substr(2);
	namesp = regex_replace(namesp,std::regex("::"),"/");
	std::string path = rootPath.append(namesp);

	return path;
}

/*
Creates the directory structure and returns relative path of file as in the repo
*/
template<typename T>
inline std::string RepositoryCore<T>::createDirectoryStructure(std::string namespaces) {
	std::string path = createPathFromNameSpace(namespaces);
	bool result = FileSystem::Directory::exists(path.substr(0,path.find_last_of("/")));
	if (result)
		return path;

	std::size_t dirUpto = 0;
	std::size_t found = path.find_last_of("/");
	dirUpto = directoryExists(path.substr(0, found));
	std::cout << "Dir exists upto : " << path.substr(0, dirUpto) << "\n";
	std::cout << "Dir to create upto : " << path.substr(dirUpto + 1) << "\n";
	std::string remainder = path.substr(dirUpto + 1, path.substr(dirUpto + 1).find_last_of("/") + 1);	 // get ride of the file name
	createDirectories(path, remainder, dirUpto);

	return path;
}

/*
Initiates the checkout process, @args requried are filename , optional @args version , instead pass 0
in case of 0 version the latest version of the file is checkout with dependencies
*/
template<typename T>
inline std::vector<std::string> RepositoryCore<T>::checkoutFile(std::string filename, std::size_t version)
{
	std::string key = getKeyByFilename(filename,version);
	vector<std::string> resultKeys;
	if (key == "") {
		cout << "\n Checkout Failed : " << filename << " does not exists in the Repository\n";
	}
	else {
		resultKeys = extractFiles(key,getDB());
		Checkout::copyToStage(getDB(), resultKeys , "../Storage"); //"../stage"
	}

	return resultKeys;
}

/*
Custom Query to fetch the files by category And Dependencies
*/
template<typename T>
inline std::vector<std::string> RepositoryCore<T>::execQuery(std::string category, std::string dependency , bool noParent) {
	vector<std::string> resultKeys;
	vector<std::string> catKeys;
	vector<std::string> depKeys;
	Query<PayLoad> q1(getDB());
	if (noParent) {
		auto hasParent = [](DbElement<PayLoad>& elem) {
			if (elem.children().empty())
				return true;
			else
				return false;
		};
		resultKeys = q1.select(hasParent).keys();
		if (!resultKeys.empty())
			resultKeys.erase(resultKeys.begin());
		return resultKeys;
	}
	if (category != "") {
		auto hasCategory = [&category](DbElement<PayLoad>& elem) {
			return (elem.payLoad()).hasCategory(category);
		};
		catKeys = q1.select(hasCategory).keys();
	}
	if (dependency != "") {
		Keys key;
		key.push_back(dependency);
		Conditions<PayLoad> conds;
		conds.children(key);
		depKeys = q1.select(conds).keys();
	}
	if (dependency == "" && depKeys.empty())
		return catKeys;
	if (category == "" && catKeys.empty())
		return depKeys;

	for (size_t i = 0; i < depKeys.size(); i++) {

		if (contains(catKeys, depKeys[i])) {
			resultKeys.push_back(depKeys[i]);
		}
	}
	return resultKeys;
}
/*
queries the database to obtain the keys that match the regex
used to get the key of the file using the filname
*/
template<typename T>
inline std::vector<std::string> RepositoryCore<T>::getKeys(std::string filename)
{
	std::string regex = ".*";
	regex = regex.append(filename).append(".*");
	Query<T> q(getDB());
	std::vector<std::string> keySet = q.selectKeys(regex).keys();

	return keySet;
}

/*
fetch the key from the db using the filename provided
*/
template<typename T>
inline std::string RepositoryCore<T>::getKeyByFilename(std::string filename, std::size_t version)
{
	vector<std::string> keys = getKeys(filename);

	if (keys.empty()) {
		return "";  //check if we can send null_ptr
	}
	else {
		if (version == 0) {
			Version v;
			version = v.getFileVersion(keys);
		}
		std::string key = keys[0];
		key = key.substr(0, key.find_last_not_of(".")).append(std::to_string(version));
		return key;
	}
}

/*
@rgs List of Dbelements ,Files are received as a batch and are processed one at a time
the method initiates the checkin process, ensures the directory structure is created,
fetches the latest version of the file and invokes sumbmit();
*/
template<typename T>
inline DbCore<T>& RepositoryCore<T>::submitFiles(std::vector<DbElement<T>> checkInElements)
{
	for (size_t i = 0; i < checkInElements.size(); i++) {
		
		std::string username = checkInElements[i].name();
		if (authenticateUser(username)) {

			std::string filename = checkInElements[i].payLoad().value();
			filename = filename.substr(filename.find_last_of("/") + 1);
			std::string namespaces = checkInElements[i].payLoad().getNamespace();
			std::string key = namespaces.append(filename);

			size_t version = getFileVersion(key);				// version of the file to be checkedin
			std::string repoPath = createDirectoryStructure(namespaces);		// repoPath is the destination for checkin

			// checkin , remember to increment the version
			Checkin::submit(getDB(), key, checkInElements[i], version, repoPath);
		}
	}
	
	return getDB();
}

/*
returns a list of transitive dependencies given the key
*/
template<typename T> 
inline std::vector<std::string> RepositoryCore<T>::extractFiles(std::string key , DbCore<T>& db)
{
	std::vector<std::string> resultSet;
	resultSet.push_back(key);
	size_t size = resultSet.size();
	for (size_t i = 0; i < size; i++) {

		std::vector<std::string> children = db[resultSet[i]].children();
		resultSet = copyValues(children, resultSet);
		size = resultSet.size();

	}

	return resultSet;
}

/*
utility function to copy two vectors
*/
template<typename T>
inline std::vector<std::string> RepositoryCore<T>::copyValues(std::vector<std::string> src, std::vector<std::string> des)
{
	for (size_t i = 0; i < src.size(); i++) {
		if (!contains(des, src[i]))
			des.push_back(src[i]);
	}
	return des;
}

/*
returns true if the vector contains the key else false
*/
template<typename T>
inline bool RepositoryCore<T>::contains(std::vector<std::string> src, std::string key)
{
	for (size_t i = 0; i < src.size(); i++) {
		if (src[i] == key)
			return true;
	}
	return false;
}

/*
Initiates the browsing process, @args requried are filename , optional @args version , instead pass 0
in case of 0 version the latest version of the file is checkout with dependencies
*/
template<typename T>
inline vector<std::string> RepositoryCore<T>::browseRepo(std::string filename, std::size_t version)
{
	vector<std::string> resultKeys;

	std::string key = getKeyByFilename(filename, version);
	if (key == "") {
		cout << "\n Browsing Failed : " << filename << " does not exists in the Repository\n";
	}
	else {
		resultKeys = extractFiles(key, getDB());
		Browse::browseFiles(getDB(),resultKeys);
	}

	return resultKeys;
}

/*
returns true if the @arg username is the administrator of the code repo , else false
*/
template<typename T>
inline bool RepositoryCore<T>::authenticateUser(std::string username) {
	DbCore<T>& db = getDB();

	if (db["root"].name() == username)
		return true;

	std::cout << "\n\n User " << username << " not authorized to perform requested operation";
	return false;
}