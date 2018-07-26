#include "Checkout.h"
#include "../NoSQLDB/PayLoad/PayLoad.h"


/////////////////////////////////////////////////////////////////////
// Checkout.cpp  - This file provides with the Test Stub		   //
// Operations Performed :copy files from Repo to the User Space	   //
// ver 1.0                                                         //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018  //
/////////////////////////////////////////////////////////////////////


Checkout::Checkout()
{
}


Checkout::~Checkout()
{
}

#ifdef TEST_CHECK_OUT

int main()
{
	using namespace std;
	using namespace NoSqlDb;

	DbCore<PayLoad> db;
	DbElement<PayLoad> newElement;
	vector<std::string> keys_to_checkout;
	
	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("file1.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../root/namespace1/namespace2/file1.cpp.1");
	newElement.payLoad().setCheckinStatus(open);
	newElement.payLoad().setNamespace("::namespace1::namespace2::");
	db["::namespace1::namespace2::file1.cpp.1"] = newElement;

	keys_to_checkout.push_back("::namespace1::namespace2::file1.cpp.1");

	Checkout::copyToStage(db, keys_to_checkout, "../stage/file1.cpp");

	return 0;
}

#endif
