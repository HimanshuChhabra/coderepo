#ifndef PAYLOAD_H
#define PAYLOAD_H
///////////////////////////////////////////////////////////////////////
// PayLoad.h - application defined payload                           //
// ver 1.1                                                           //
// author : Himanshu Chhabra, CSE687                                 //
// source : Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018//
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  This package provides a single class, PayLoad:
*  - holds a payload string and vector of categories
*  - provides means to set and access those values
*  - provides methods used by Persist<PayLoad>:
*    - Sptr toXmlElement();
*    - static PayLoad fromXmlElement(Sptr elem);
*  - provides a show function to display PayLoad specific information
*  - PayLoad processing is very simple, so this package contains only
*    a header file, making it easy to use in other packages, e.g.,
*    just include the PayLoad.h header.
*
*  Required Files:
*  ---------------
*    PayLoad.h, PayLoad.cpp - application defined package
*    DbCore.h, DbCore.cpp
*
*  Maintenance History:
*  --------------------
*  ver 1.1 : 19 Feb 2018
*  - added inheritance from IPayLoad interface
*  Ver 1.0 : 10 Feb 2018
*  - first release
*
*/

#include <string>
#include <vector>
#include <iostream>
/*
#include "../XmlDocument/XmlDocument/XmlDocument.h"
#include "../XmlDocument/XmlElement/XmlElement.h"
#include "../DbCore/Definitions.h"
#include "../DbCore/DbCore.h"
*/

//#include "../../NoSQLDB/XmlDocument/XmlDocument/XmlDocument.h"
//#include "../../NoSQLDB/XmlDocument/XmlElement/XmlElement.h"
//#include "../../NoSQLDB/DbCore/Definitions.h"
//#include "../../NoSQLDB/DbCore/DbCore.h"

#include "../../NoSQLDB/Executive/NoSqlDb.h"
#include "IPayLoad.h"

///////////////////////////////////////////////////////////////////////
// PayLoad class provides:
// - a std::string value which, in Project #2, will hold a file path
// - a vector of string categories, which for Project #2, will be 
//   Repository categories
// - methods used by Persist<PayLoad>:
//   - Sptr toXmlElement();
//   - static PayLoad fromXmlElement(Sptr elem);
//  - enum state - mainatains the checkin status of the files in code repo


namespace NoSqlDb
{
	enum state
	{
		open,
		closing,
		closed
    };

  class PayLoad : public IPayLoad<PayLoad>
  {
  public:
    PayLoad() = default;
    PayLoad(const std::string& val) : value_(val) {}
    static void identify(std::ostream& out = std::cout);
    PayLoad& operator=(const std::string& val)
    {
      value_ = val;
      return *this;
    }
    operator std::string() { return value_; }

    std::string value() const { return value_; }
    std::string& value() { return value_; }
    void value(const std::string& value) { value_ = value; }

    std::vector<std::string>& categories() { return categories_; }
    std::vector<std::string> categories() const { return categories_; }

    bool hasCategory(const std::string& cat)
    {
      return std::find(categories().begin(), categories().end(), cat) != categories().end();
    }

    Sptr toXmlElement();
    static PayLoad fromXmlElement(Sptr elem);

    static void showPayLoadHeaders(std::ostream& out = std::cout);
    static void showElementPayLoad(NoSqlDb::DbElement<PayLoad>& elem, std::ostream& out = std::cout);
    static void showDb(NoSqlDb::DbCore<PayLoad>& db, std::ostream& out = std::cout);

	void setCheckinStatus(state value) { checkinStatus = value; }
	state getCheckinStatus() const { return checkinStatus; }
	state& getCheckinStatus() { return checkinStatus; }

	void setNamespace(std::string value) { namespace_ = value; }
	std::string getNamespace() const { return namespace_; }
	std::string getNamespace() { return namespace_; }
  private:
    std::string value_;
    std::vector<std::string> categories_;
	state checkinStatus;
	std::string namespace_;

  };

  //----< show file name >---------------------------------------------

  inline void PayLoad::identify(std::ostream& out)
  {
    out << "\n  \"" << __FILE__ << "\"";
  }
  //----< create XmlElement that represents PayLoad instance >---------
  /*
  * - Required by Persist<PayLoad>
  */
  inline Sptr PayLoad::toXmlElement()
  {
    Sptr sPtr = XmlProcessing::makeTaggedElement("payload");
    XmlProcessing::XmlDocument doc(makeDocElement(sPtr));
    Sptr sPtrVal = XmlProcessing::makeTaggedElement("value",value_);
    sPtr->addChild(sPtrVal);
	Sptr sPtrNamespace = XmlProcessing::makeTaggedElement("namespace", namespace_);
	sPtr->addChild(sPtrNamespace);
	Sptr sPtrStatus = XmlProcessing::makeTaggedElement("status", std::to_string(checkinStatus));
	sPtr->addChild(sPtrStatus);
    Sptr sPtrCats = XmlProcessing::makeTaggedElement("categories");
    sPtr->addChild(sPtrCats);
    for (auto cat : categories_)
    {
      Sptr sPtrCat = XmlProcessing::makeTaggedElement("category", cat);
      sPtrCats->addChild(sPtrCat);
    }
    return sPtr;
  }
  //----< create PayLoad instance from XmlElement >--------------------
  /*
  * - Required by Persist<PayLoad>
  */
  inline PayLoad PayLoad::fromXmlElement(Sptr pElem)
  {
    PayLoad pl;
    for (auto pChild : pElem->children())
    {
      std::string tag = pChild->tag();
      std::string val;
	  if (pChild->children().size() > 0)
		  val = pChild->children()[0]->value();
	  else
		  val = "";
      if (tag == "value")
      {
        pl.value(val);
      }
	  if (tag == "namespace")
	  {
		  pl.setNamespace(val);
	  }
	  if (tag == "status")
	  {
		  if (val == "0") {
			  pl.setCheckinStatus(open);
		  }
		  else if (val == "2") {
			  pl.setCheckinStatus(closed);
		  }
		  else if (val == "1") {
			  pl.setCheckinStatus(closing);
		  }
	  }
      if (tag == "categories")
      {
        std::vector<Sptr> pCategories = pChild->children();
        for (auto pCat : pCategories)
        {
          pl.categories().push_back(pCat->children()[0]->value());
        }
      }
    }
    return pl;
  }
  /////////////////////////////////////////////////////////////////////
  // PayLoad display functions

  inline void PayLoad::showPayLoadHeaders(std::ostream& out)
  {
    out << "\n  ";
    out << std::setw(25) << std::left << "Name";
    out << std::setw(50) << std::left << "Payload Value";
	out << std::setw(35) << std::left << "Checkin Status";
	out << std::setw(45) << std::left << "Namespace";
    out << std::setw(25) << std::left << "Categories";
    out << "\n  ";
    out << std::setw(25) << std::left << "---------------";
    out << std::setw(50) << std::left << "----------------------------";
	out << std::setw(35) << std::left << "---------------";
	out << std::setw(40) << std::left << "--------------------------------------";
    out << std::setw(25) << std::left << "-----------------------";
  }


  inline void PayLoad::showElementPayLoad(NoSqlDb::DbElement<PayLoad>& elem, std::ostream& out)
  {
    out << "\n  ";
    out << std::setw(25) << std::left << elem.name();
    out << std::setw(50) << std::left << elem.payLoad().value().substr(0,48);
	out << std::setw(35) << std::left << elem.payLoad().getCheckinStatus();
	out << std::setw(40) << std::left << elem.payLoad().getNamespace().substr(0,38);
    for (auto cat : elem.payLoad().categories())
    {
      out << cat << " ";
    }
  }

  inline void PayLoad::showDb(NoSqlDb::DbCore<PayLoad>& db, std::ostream& out)
  {
    showPayLoadHeaders(out);
    for (auto item : db)
    {
      NoSqlDb::DbElement<PayLoad> temp = item.second;
      PayLoad::showElementPayLoad(temp, out);
    }
  }
}
#endif
