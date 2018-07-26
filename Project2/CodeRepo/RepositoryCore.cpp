#include "RepositoryCore.h"

/////////////////////////////////////////////////////////////////////
// RepositoryCore.h - provides operations and utilities for		   //
// checkin, checkout, browse operations							   //
// Operations Performed :Invokes the Test stubs					   //
// ver 1.0                                                         //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018  //
/////////////////////////////////////////////////////////////////////



#ifdef TEST_REPO

int main()
{
	using namespace std;
	using namespace NoSqlDb;

	DbCore<PayLoad> db;
	std::vector<DbElement<PayLoad>> elementList;

	DbElement<PayLoad> newElement;

	newElement.name("Himanshu");
	newElement.descrip("file3.h file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file3.h");					//user will give path from /dir1/dir2 ..
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace4::namespace5::");
	newElement.children().clear();
	elementList.push_back(newElement);

	newElement.name("Himanshu");
	newElement.descrip("file3.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file3.cpp");					//user will give path from /dir1/dir2 ..
	newElement.payLoad().setCheckinStatus(open);
	newElement.payLoad().setNamespace("::namespace4::namespace5::");
	newElement.children().push_back("::namespace4::namespace5::file3.h.1");
	elementList.push_back(newElement);

	RepositoryCore<PayLoad> r(db);
	db = r.submitFiles(elementList);
	r.checkoutFile("::namespace4::namespace5::file3.cpp",0);	// version set to 0, this defaults to latest version
	r.browseRepo("::namespace4::namespace5::file3.cpp", 0);
	
	PayLoad::showDb(db);

	return 0;
}

#endif
