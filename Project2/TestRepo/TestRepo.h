#pragma once
#include "../CodeRepo/RepositoryCore.h"
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../NoSQLDB/PayLoad/PayLoad.h"
#include "../NoSQLDB/Persist/Persist.h"
#include <iomanip>


/////////////////////////////////////////////////////////////////////
// TestRepo.cpp - Test Engine										 //
// Operations Performed :Invokes the Test stubs					   //
// ver 1.0                                                         //
// Himanshu Chhabra, CSE687 - Object Oriented Design, Spring 2018  //
/////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package provides
* - This package contains the test stubs

* Required Files:
* ---------------
- RepositoryCore.h , RepositoryCore.cpp
- NoSqlDb.h , NoSqlDb.cpp
- Persist.h , Persist.cpp

*
* Maintenance History:
* --------------------
* ver 1.0 : 08 March 2018
*/

class TestRepo
{
public:
	TestRepo();
	~TestRepo();
};

//bool testR1();