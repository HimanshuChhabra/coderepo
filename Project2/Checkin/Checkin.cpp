#include "Checkin.h"
#include "../NoSQLDB/PayLoad/PayLoad.h"

//#include "../Process/Process/Process.h"
/////////////////////////////////////////////////////////////////////
// Checkin.cpp - Contains Test Stubs 							   //
// Operations Performed :copy files from Stage area to the repo	   //
// ver 1.0                                                         //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018  //
/////////////////////////////////////////////////////////////////////


std::unordered_map<Key, vector<std::string>> Checkin::dependentOn = {};
std::unordered_map<Key, vector<std::string>> Checkin::parentOf = {};

Checkin::Checkin()
{
}


Checkin::~Checkin()
{
}

#ifdef TEST_CHECKIN


int main()
{
	using namespace std;
	using namespace NoSqlDb;

	DbCore<PayLoad> db;
	DbElement<PayLoad> newElement;

	newElement.name("Himanshu");
	newElement.descrip("file3.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file3.cpp");					
	newElement.payLoad().setCheckinStatus(open);
	newElement.payLoad().setNamespace("::name1::name2::");
	newElement.children().push_back("::name1::name2::file3.h.1");
	

	Checkin::submit(db,"::name1::name2::file3",newElement,0,"../root/name1/name2/file3.cpp");
	return 0;
}

#endif
