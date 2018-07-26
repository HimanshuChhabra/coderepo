#include "TestRepo.h"

/////////////////////////////////////////////////////////////////////
// TestRepo.cpp - Test Engine										 //
// Operations Performed :Invokes the Test stubs					   //
// ver 1.0                                                         //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018  //
/////////////////////////////////////////////////////////////////////



using namespace NoSqlDb;

//----< reduce the number of characters to type >----------------------

auto putLine = [](size_t n = 1, std::ostream& out = std::cout)
{
	Utilities::putline(n, out);
};


class DbProvider
{
public:
	DbCore<PayLoad>& db() { return db_; }
private:
	static DbCore<PayLoad> db_;
};

DbCore<PayLoad> DbProvider::db_;

bool testR1() {
	Utilities::Title("Testing Core Repositiory ");
	std::cout << "\n\n NOTE: Payload Status Open = 0 , Closing (Pending) = 1 , Closed = 2";
	std::cout << "\n\n ADMIN NAME : Himanshu , has rights for checkin operation";
	Utilities::Title("\n\n\nTesting Core Repositiory -- Reading DB from XML");
	Utilities::putline();
	DbProvider dbp;
	DbCore<PayLoad> db;
	Persist<PayLoad> p(db);

	std::string readXml,inputXml;
	std::ifstream inputFile("../readXML.xml");

	while (inputFile >> readXml) {
		inputXml.append(readXml);
	}
	inputFile.close();
	bool read = p.fromXml(inputXml, true);

	if (read) {
		std::cout << "\n\n DB Successfully populated from readXML.xml \n";
		dbp.db() = db;
		std::cout << "\n\n Current Repository DB \n";
		std::cout << "--------------------------------\n";
		showDb(db);
		std::cout << "\n\n Current Repo DB Payload Status\n";
		std::cout << "--------------------------------\n";
		PayLoad::showDb(db);
	}

	return read;
}

bool testR2() {
	Utilities::Title("\n\n\nDemonstrating - Checkin two new files , file3.cpp(as Open) and file3.h (as Closed)");
	Utilities::putline();
	std::cout << "\nBoth the Files will be checked in with new version (1) \n";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;

	DbElement<PayLoad> newElement;

	newElement.name("Himanshu");
	newElement.descrip("file3.h file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file3.h");					//user will give path from /dir1/dir2 ..
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace4::namespace5::");
	newElement.payLoad().categories().push_back("Cat6");
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
	dbp.db() = r.submitFiles(elementList);
	db = dbp.db();
	//showDb(db);
	PayLoad::showDb(db);
	return true;
}

bool testR3() {
	Utilities::Title("\n\n\nDemonstrating - Checkin in file with open status ; existing file in Db has open status, checkin file1.cpp");
	Utilities::putline();
	std::cout << "\n\n As file1.cpp.1 has a checkin status as open , current checkin will not create any new version, but will overwrite";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;
	std::cout << "\n\n Before checking in file1.cpp\n";
	showHeader();
	showRecord("::namespace1::namespace2::file1.cpp.1",db["::namespace1::namespace2::file1.cpp.1"]);

	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("updated file1.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file1.cpp");					
	newElement.payLoad().setCheckinStatus(open);
	newElement.payLoad().setNamespace("::namespace1::namespace2::");
	newElement.children().push_back("::namespace1::namespace2::file1.h.1");
	newElement.payLoad().categories().push_back("Cat5");
	newElement.payLoad().categories().push_back("Cat2");
	elementList.push_back(newElement);

	RepositoryCore<PayLoad> r(db);
	dbp.db() = r.submitFiles(elementList);
	std::cout << "\n\n After checking in file1.cpp\n";
	db = dbp.db();
	showHeader();
	showRecord("::namespace1::namespace2::file1.cpp.1", db["::namespace1::namespace2::file1.cpp.1"]);
	return true;
}

bool testR4() {
	Utilities::Title("\n\n\nDemonstrating - Checkin in file with open status ; existing file in Db has Closed status, checkin file2.cpp");
	Utilities::putline();
	std::cout << "\n\n As file2.cpp.1 has a checkin status as closed , current checkin will create a new version(2) of file2.cpp";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;
	
	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("updated file2.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file2.cpp");
	newElement.payLoad().setCheckinStatus(open);
	newElement.payLoad().setNamespace("::namespace3::");
	newElement.children().push_back("::namespace3::file2.h.1");
	newElement.payLoad().categories().push_back("Cat4");
	newElement.payLoad().categories().push_back("Cat6");
	elementList.push_back(newElement);

	RepositoryCore<PayLoad> r(db);
	dbp.db() = r.submitFiles(elementList);
	std::cout << "\n\n After Submitting file2.cpp";
	db = dbp.db();
	std::cout << "\n\n Versions of file file2.cpp\n";
	PayLoad::showPayLoadHeaders();
	
	PayLoad::showElementPayLoad(db["::namespace3::file2.cpp.1"]);
	PayLoad::showElementPayLoad(db["::namespace3::file2.cpp.2"]);
	return true;
}

bool testR5() {	// file 1 is waiting for 3.1 and 2.2
	Utilities::Title("\n\n\nDemonstrating - Checkin in file with Closed status ; existing file in Db has an Open status, checkin file1.cpp");
	Utilities::putline();
	std::cout << "\n\n As file1.cpp.1 has a checkin status as open , current checkin will not create any new version, but will overwrite";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;
	std::cout << "\n\n Before checking in file1.cpp , payload status:\n";
	PayLoad::showPayLoadHeaders();
	PayLoad::showElementPayLoad(db["::namespace1::namespace2::file1.cpp.1"]);

	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("new file1.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file1.cpp");
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace1::namespace2::");
	newElement.children().push_back("::namespace1::namespace2::file1.h.1");
	newElement.children().push_back("::namespace3::file2.cpp.2");
	newElement.children().push_back("::namespace4::namespace5::file3.cpp.1");
	newElement.payLoad().categories().push_back("Cat1");
	//newElement.children().push_back("::namespace6::file4.cpp.1");
	elementList.push_back(newElement);

	RepositoryCore<PayLoad> r(db);
	dbp.db() = r.submitFiles(elementList);
	db = dbp.db();
	std::cout << "\n\n After checking in file1.cpp , payload status:\n";
	PayLoad::showPayLoadHeaders();
	PayLoad::showElementPayLoad(db["::namespace1::namespace2::file1.cpp.1"]);

	return true;
}

bool testR6() { // for 1.1 , 2.2 , 3.1
	Utilities::Title("\n\n\nDemonstrating - Checkin in file with Closed status ; existing file in Db has an Open status, checkin file4.cpp");
	Utilities::putline();
	std::cout << "\n\n As file4.cpp.1 has a checkin status as open , current checkin will not create any new version, but will overwrite";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;
	std::cout << "\n\n Before checking in file4.cpp , payload status:\n";
	PayLoad::showPayLoadHeaders();
	PayLoad::showElementPayLoad(db["::namespace6::file4.cpp.1"]);

	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("new file4.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file4.cpp");
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace6::");
	newElement.children().push_back("::namespace1::namespace2::file1.cpp.1");
	newElement.children().push_back("::namespace6::file4.h.1");
	newElement.payLoad().categories().push_back("Cat2");
	elementList.push_back(newElement);

	RepositoryCore<PayLoad> r(db);
	dbp.db() = r.submitFiles(elementList);
	std::cout << "\n\n Note: file4.cpp is dependent on file1.cpp.1 and file1.cpp.1 is dependent on file2.cpp.2 and file3.cpp.1";
	std::cout << "\n file4.cpp is transitively dependent on file2.cpp2 and file3.cpp.1  \n";
	db = dbp.db();
	std::cout << "\n\n After checking in file4.cpp , payload status:\n";
	PayLoad::showPayLoadHeaders();
	PayLoad::showElementPayLoad(db["::namespace6::file4.cpp.1"]);

	return true;
}

bool testR7() {
	Utilities::Title("\n\n\nDemonstrating - Closing Dependency files; file3.cpp.1 and file2.cpp.2");
	Utilities::putline();
	std::cout << "\n\n file1.cpp.1 and file4.cpp.1 will auto close after file3.cpp.1 and file2.cpp.2 are closed";
	std::cout << "\n\n As file3.cpp.1 and file2.cpp.2  have checkin status as open , current checkin will not create any new version, but will overwrite";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;
	std::cout << "\n\n Before closing file3.cpp.1 and file2.cpp.2 , payload status:";
	PayLoad::showPayLoadHeaders();
	PayLoad::showElementPayLoad(db["::namespace4::namespace5::file3.cpp.1"]);
	PayLoad::showElementPayLoad(db["::namespace3::file2.cpp.2"]);
	PayLoad::showElementPayLoad(db["::namespace1::namespace2::file1.cpp.1"]);
	PayLoad::showElementPayLoad(db["::namespace6::file4.cpp.1"]);
	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("new file3.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file3.cpp");
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace4::namespace5::");
	newElement.children().push_back("::namespace4::namespace5::file3.h.1");
	newElement.payLoad().categories().push_back("Cat5");
	elementList.push_back(newElement);
	newElement.name("Himanshu");
	newElement.descrip("new file2.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file2.cpp");
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace3::");
	newElement.children().clear();
	newElement.children().push_back("::namespace3::file2.h.1");
	newElement.payLoad().categories().push_back("Cat6");
	elementList.push_back(newElement);
	RepositoryCore<PayLoad> r(db);
	dbp.db() = r.submitFiles(elementList);
	db = dbp.db();
	std::cout << "\n\n After closing file3.cpp.1 and file2.cpp.2 , payload status:";
	PayLoad::showPayLoadHeaders();
	PayLoad::showElementPayLoad(db["::namespace4::namespace5::file3.cpp.1"]);
	PayLoad::showElementPayLoad(db["::namespace3::file2.cpp.2"]);
	PayLoad::showElementPayLoad(db["::namespace1::namespace2::file1.cpp.1"]);
	PayLoad::showElementPayLoad(db["::namespace6::file4.cpp.1"]);
	return true;
}

bool testR11() { // for 1.1 , 2.2 , 3.1
	Utilities::Title("\n\nDemonstrating - Checkin in file with Closed status ; existing file in Db has an Closed status, checkin file4.cpp");
	Utilities::putline();
	std::cout << "\n\n file4.cpp.1 has a checkin status as closed , current checkin will  create a new version";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;
	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("new file4.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file4.cpp");
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace6::");
	newElement.children().push_back("::namespace1::namespace2::file1.cpp.1");
	newElement.children().push_back("::namespace6::file4.h.1");
	newElement.payLoad().categories().push_back("Cat3");
	elementList.push_back(newElement);
	RepositoryCore<PayLoad> r(db);
	dbp.db() = r.submitFiles(elementList);
	std::cout << "\n\n After Submitting file4.cpp";
	db = dbp.db();
	std::cout << "\n\n Versions of file file4.cpp\n";
	PayLoad::showPayLoadHeaders();
	PayLoad::showElementPayLoad(db["::namespace6::file4.cpp.1"]);
	PayLoad::showElementPayLoad(db["::namespace6::file4.cpp.2"]);
	return true;
}
bool testR8() {
	Utilities::Title("\n\nDemonstrating - Checkout a file to staging area");
	Utilities::putline();
	std::cout << "\n\n Checkout file4.cpp.1 to location ../stage/";
	std::cout << "\n\n Note:: If user does not specify the version , Latest version from Repo is checkedout ";
	std::cout << "\n\n Note:: Files during checkout will overwrite the files with same name";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();

	RepositoryCore<PayLoad> r(db);
	r.checkoutFile("::namespace6::file4.cpp", 1);

	return true;
}

bool testR9() {
	Utilities::Title("\n\nDemonstrating - Browsing a file ");
	Utilities::putline();
	std::cout << "\n\n Browsing file file2.cpp.2 from Repository";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	RepositoryCore<PayLoad> r(db);
	r.browseRepo("::namespace3::file2.cpp",2);
	db = dbp.db();
	return true;
}

bool testR10() {
	Utilities::Title("\n\nDemonstrating - Admin rights, invalid username restricted checkin");
	Utilities::putline();
	std::cout << "\n\n Checkin file2.cpp with invalid username ABC";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;
	DbElement<PayLoad> newElement;
	newElement.name("ABC");
	newElement.descrip("latest file2.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file2.cpp");
	newElement.payLoad().setCheckinStatus(open);
	newElement.payLoad().setNamespace("::namespace3::");
	newElement.children().push_back("::namespace3::file2.h.1");
	newElement.payLoad().categories().push_back("Cat5");
	elementList.push_back(newElement);
	RepositoryCore<PayLoad> r(db);
	r.submitFiles(elementList);
	db = dbp.db();
	return true;
}

bool testR12() {
	Utilities::Title("\n\nDemonstrating - Persisting the database into xml file at ../persistedXML.xml");
	Utilities::putline();
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	Persist<PayLoad> p(db);
	std::string xml = p.toXml();
	std::ofstream outputFile("../persistedXML.xml");
	outputFile << xml;
	outputFile.close();
	std::cout << "\n\n Repo DB Persisted in xml - ../persistedXML.xml";
	std::cout << "\n\n Note: For Demo Purpose The DB is persisted in a separate XML file for comparison and verification";
	std::cout << "\n\n Ideally It should overwrite the readXML, such that when DB is loaded again it loads previously Stored Data";
	std::cout << "\n\n To Load previously stored Data rename ../persistedXML.xml to ../readXML.xml";
	std::cout << "\n\n However it is not recommended to do so as Test Cases are designed to operate on original readXML.xml";
	Utilities::Title("\n\n Final Code Repo Db ");
	Utilities::putline();
	showDb(db);
	Utilities::Title("\n\n Final Code Repo Db Payload Status ");
	Utilities::putline();
	PayLoad::showDb(db);

	return true;

}

bool testR13() {
	Utilities::Title("\n\nDemonstrating - Demonstrating circular dependencies resolution");
	Utilities::putline();
	std::cout << "\n\nChecking in two new files , file6.cpp and file6.h which are dependent on each other";
	std::cout << "\n\n 1. Checkin file6.h as open. \n 2. Checkin file6.cpp as closed dependent on file.6.h.1 created in step 1.";
	std::cout << "\n 3. Checkin file6.h as closed dependent on file6.cpp.1 created in step2.";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	std::vector<DbElement<PayLoad>> elementList;
	DbElement<PayLoad> newElement;
	newElement.name("Himanshu");
	newElement.descrip("file6.h file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file6.h");
	newElement.payLoad().setCheckinStatus(open);
	newElement.payLoad().setNamespace("::namespace7::");
	newElement.payLoad().categories().push_back("Cat5");
	elementList.push_back(newElement);
	newElement.name("Himanshu");
	newElement.descrip("file6.cpp file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file6.cpp");
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace7::");
	newElement.children().push_back("::namespace7::file6.h.1");
	elementList.push_back(newElement);
	newElement.name("Himanshu");
	newElement.descrip("file6.h file");
	newElement.dateTime(DateTime().now());
	newElement.payLoad().value("../stage/file6.h");
	newElement.payLoad().setCheckinStatus(closed);
	newElement.payLoad().setNamespace("::namespace7::");
	newElement.children().clear();newElement.children().push_back("::namespace7::file6.cpp.1");
	elementList.push_back(newElement);
	RepositoryCore<PayLoad> r(db);
	std::cout << "\n\nAfter Checkin:";
	dbp.db() = r.submitFiles(elementList);
	std::cout << "\n\n After Checkin, Since file6.cpp.1 was already waiting for file6.h.1 ; file6.h.1 before closing notified file6.cpp.1 and closed itself\n";
	std::cout << "\n\n Resulting in No Circular wait - Payload Status as follows";
	db = dbp.db();
	PayLoad::showPayLoadHeaders();
	PayLoad::showElementPayLoad(db["::namespace7::file6.h.1"]);
	PayLoad::showElementPayLoad(db["::namespace7::file6.cpp.1"]);
	return true;
}

#ifdef TEST_EXEC

/*
* - This code is an example of how a client will use the NoSqlDb library.
*/
int main()
{

	TestExecutive ex;
	TestExecutive::TestStr ts1{ testR1, "\n\n Reading from Xml" };
	TestExecutive::TestStr ts2{ testR2, "\n\n Adding new Files to xml, one as Open Checkin and other as Closed Checkin status" };
	TestExecutive::TestStr ts3{ testR3, "\n\n Checkin in file with Open status ; existing file in Db has Open status" };
	TestExecutive::TestStr ts4{ testR4, "\n\n Checkin in file with Open status ; existing file in Db has Closed status" };
	TestExecutive::TestStr ts5{ testR5, "\n\n Checkin in file with Closed status ; existing file in Db has Open status" };
	TestExecutive::TestStr ts6{ testR6, "\n\n Checkin in file with Closed status ; existing file in Db has Open status - file4.cpp" };
	TestExecutive::TestStr ts7{ testR7, "\n\n Closing Dependency files" };
	TestExecutive::TestStr ts11{ testR11, "\n\n Checkin in file with Closed status ; existing file in Db has Closed status - file4.cpp" };
	TestExecutive::TestStr ts8{ testR8, "\n\n Checkout a file to staging area" };
	TestExecutive::TestStr ts9{ testR9, "\n\n Browsing a file" };
	TestExecutive::TestStr ts10{ testR10, "\n\n Admin rights, invalid username restricted checkin" };
	TestExecutive::TestStr ts12{ testR12, "\n\n Persisting the database into xml" };
	TestExecutive::TestStr ts13{ testR13, "\n\n Demonstrating - Demonstrating circular dependencies resolution" };
	ex.registerTest(ts1);
	ex.registerTest(ts2);
	ex.registerTest(ts3);
	ex.registerTest(ts4);
	ex.registerTest(ts5);
	ex.registerTest(ts6);
	ex.registerTest(ts7);
	ex.registerTest(ts11);
	ex.registerTest(ts8);
	ex.registerTest(ts9);
	ex.registerTest(ts10);
	ex.registerTest(ts13);
	ex.registerTest(ts12);
	
	// run tests
	bool result = ex.doTests();
	if (result == true)
		std::cout << "\n  all tests passed";
	else
		std::cout << "\n  at least one test failed";
	getchar();
	return 0;
}

#endif
