#pragma once
#include <vector>
#include "../NoSQLDB/Executive/NoSqlDb.h"

///////////////////////////////////////////////////////////////////////
// version.h													     //
// Operations Performed: Handles the versioning  aspect of the       //
// 	repository                             			                 //	
// ver 1.0                                                           //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018    //
///////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package provides
* - This package provides mechanism to calcualte the version of the files in the repo
	- getFileVersion() - returns the Latest versio of the file in the repo 
	- The method returns 0 in case of a non existing file/new file.
	- Method is provided with the repo db access along with the file for 
	  which  latest version is to be obtained

* Required Files:
* ---------------
- NoSqlDb.h , NoSqlDb.cpp

*
* Maintenance History:
* --------------------
* ver 1.0 : 08 March 2018
*/


class Version
{
public:
	Version() = default;
    std::size_t getFileVersion(std::vector<std::string> keys);
private :
	static std::size_t getMaxVersion(const std::vector<std::string>& keys);

};

/*
Returns the existing version of the file.If file is new, it returns 0
*/
inline std::size_t Version::getFileVersion(std::vector<std::string> keys) {

	if (keys.empty()) {
		return 0;
	}

	return getMaxVersion(keys);
}