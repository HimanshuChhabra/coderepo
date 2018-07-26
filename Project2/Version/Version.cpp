#include "Version.h"
#include <vector>


///////////////////////////////////////////////////////////////////////
// version.cpp - Implmentation file									 //
// Operations Performed: Handles the versioningaspect of the repository//
// 									 //	
// ver 1.0                                                           //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018    //
///////////////////////////////////////////////////////////////////////

/*
- It is the implementation file
- Provides Test Stub
*/

// calculates the max version of the files passed as args
std::size_t Version::getMaxVersion(const std::vector<std::string>& keys) {

	size_t max = 0;

	for (auto key : keys) {
		size_t found = key.find_last_of(".");
		std::string version = key.substr(found + 1);
		std::stringstream stream(version);
		size_t version_ = 0;
		stream >> version_;
		if (max < version_) {
			max = version_;
		}
	}
	return max;
}



#ifdef TEST_VERSION

int main()
{
	using namespace std;

	vector<std::string> key;

	key.push_back("::namespace1::file1.cpp.1");
	key.push_back("::namespace1::file1.cpp.2");
	key.push_back("::namespace1::file1.cpp.3");

	cout << "\n\n Existing File Latest version : " << Version::getFileVersion(key);
	return 0;
}

#endif
