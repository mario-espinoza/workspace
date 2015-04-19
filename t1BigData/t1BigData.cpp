//============================================================================
// Name        : t1BigData.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <utility>
#include <string.h>
#include "pugixml.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/xml_iarchive.hpp>

using namespace std;
using namespace boost;

struct letter_only: std::ctype<char>
{
    letter_only(): std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table()
    {
        static std::vector<std::ctype_base::mask>
            rc(std::ctype<char>::table_size,std::ctype_base::space);

        std::fill(&rc['A'], &rc['z'+1], std::ctype_base::alpha);
        return &rc[0];
    }
};

/*class Badges
{
public:
	Badges()
	{

	};
	void SetRow(const std::string& name, const std::string value)
	{
		rows[name] = value;
	}
private:
	int Id;
	int UserId;
	string Name;
	string Date;

	map< string, string > rows;
	friend class boost::serialization::access;

	template<class archive>

	void serialize(archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("row",rows);
		ar & BOOST_SERIALIZATION_NVP(Id);
		ar & BOOST_SERIALIZATION_NVP(UserId);
		ar & BOOST_SERIALIZATION_NVP(Name);
		ar & BOOST_SERIALIZATION_NVP(Date);
	}
};
*/
int main( int argc, char ** argv )
{

	std::map<std::string, int> wordCount;
	ifstream input;
	input.imbue(std::locale(std::locale(), new letter_only())); //enable reading only letters!
	input.open("filename.txt");
	std::string word;
	while(input >> word)
	{
		++wordCount[word];
	}
	for (std::map<std::string, int>::iterator it = wordCount.begin(); it != wordCount.end(); ++it)
	{
		cout << it->first <<" : "<< it->second << endl;
	}


	pugi::xml_document doc;

	    pugi::xml_parse_result result = doc.load_file("badges.xml");

	    //std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("badges").child("row").attribute("Name").value() << std::endl;

	   // if (!doc.load_file("badges.xml")) return -1;

	   pugi::xml_node tools = doc.child("badges");
	   //cout<< tools.

	   for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it)
	   {
		   cout << "Tool:";

		   for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		   {
			   cout << " " << ait->name() << "=" << ait->value();
		   }
		   cout << endl;
	   }
	   cout << "Sali "<<endl;
	return 0;
}
