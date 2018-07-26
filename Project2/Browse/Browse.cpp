#include "Browse.h"
#include "../NoSQLDB/PayLoad/PayLoad.h"

///////////////////////////////////////////////////////////////////////
// Browse.cpp - Contains Test Stubs 							     //
// Operations Performed: Allows to view the requested file         //
// Also view its dependencies										 //	
// ver 1.0                                                           //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018    //
///////////////////////////////////////////////////////////////////////



Browse::Browse()
{
}


Browse::~Browse()
{
}

#ifdef TEST_BROWSE

int main()
{
	using namespace std;
	using namespace NoSqlDb;

	DbCore<PayLoad> db;
	DbElement<PayLoad> newElement;
	vector<std::string> keys_to_browse;

	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("file1.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../root/namespace1/namespace2/file1.cpp.1");
	newElement.payLoad().setCheckinStatus(open);
	newElement.payLoad().setNamespace("::namespace1::namespace2::");
	db["::namespace1::namespace2::file1.cpp.1"] = newElement;

	keys_to_browse.push_back("::namespace1::namespace2::file1.cpp.1");

	Browse::browseFiles(db, keys_to_browse);
	

	return 0;
}

#endif
