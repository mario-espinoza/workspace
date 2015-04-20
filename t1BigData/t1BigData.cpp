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

void loadfile(const char* filename)
{

}

int main( int argc, char ** argv )
{
	std::map<std::string, int> wordCount_sf,wordCount_wp;

	/****** leer *******/
	/*pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file("badges.xml");

	pugi::xml_node badges_test = doc.child("badges");

	for (pugi::xml_node_iterator it = badges_test.begin(); it != badges_test.end(); ++it)
	{
		//cout << "Tool:";
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			//if(ait->name()=="")
				cout << " " << ait->name() << "=" << ait->value();
		}
		cout << endl;
	}*/

	pugi::xml_document doc_posts_sf,doc_posts_wp;

	pugi::xml_parse_result result_posts_sf = doc_posts_sf.load_file("./serverfault/Posts.xml");
	pugi::xml_parse_result result_posts_wp = doc_posts_wp.load_file("./wordpress/Posts.xml");

	cout<<"Posts"<<endl;

	pugi::xml_node posts_node_sf = doc_posts_sf.child("posts");
	pugi::xml_node posts_node_wp = doc_posts_wp.child("posts");

	for (pugi::xml_node_iterator it = posts_node_sf.begin(); it != posts_node_sf.end(); ++it)
	{
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string name = ait->name();
			if(name=="Body")
			{
				stringstream ss;

				ss.imbue(std::locale(std::locale(), new letter_only())); //enable reading only letters!

				ss << ait->value();
				string word;
				while(ss >> word)
				{
					++wordCount_sf[word];
				}
			}
		}
	}

	result_posts_sf = doc_posts_sf.load_file("./serverfault/PostsHistory.xml");
	posts_node_sf = doc_posts_sf.child("posthistory");
	cout<<"History"<<endl;
	for (pugi::xml_node_iterator it = posts_node_sf.begin(); it != posts_node_sf.end(); ++it)
	{
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string name = ait->name();
			if(name=="Text")
			{
				stringstream ss;

				ss.imbue(std::locale(std::locale(), new letter_only())); //enable reading only letters!

				ss << ait->value();
				string word;
				while(ss >> word)
				{
					++wordCount_sf[word];
				}
			}
		}
	}

	result_posts_sf = doc_posts_sf.load_file("./serverfault/Comments.xml");
	posts_node_sf = doc_posts_sf.child("comments");
		cout<<"Comments"<<endl;
		for (pugi::xml_node_iterator it = posts_node_sf.begin(); it != posts_node_sf.end(); ++it)
		{
			for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
			{
				string name = ait->name();
				if(name=="Text")
				{
					stringstream ss;

					ss.imbue(std::locale(std::locale(), new letter_only())); //enable reading only letters!

					ss << ait->value();
					string word;
					while(ss >> word)
					{
						++wordCount_sf[word];
					}
				}
			}
		}

	/***** fin leer   */
	cout<<"SF"<<endl;
	ofstream csvfile_sf ("sf_frequency.csv");
	csvfile_sf << "'Word'; 'Frequency' " << endl;
	for (std::map<std::string, int>::iterator it = wordCount_sf.begin(); it != wordCount_sf.end(); ++it)
	{
		csvfile_sf << it->first <<" ; "<< it->second << endl;
	}
	csvfile_sf.close();
	cout<<"WP"<<endl;
	ofstream csvfile_wp ("wp_frequency.csv");
	csvfile_wp << "'Word'; 'Frequency' " << endl;
	for (std::map<std::string, int>::iterator it = wordCount_wp.begin(); it != wordCount_wp.end(); ++it)
	{
		csvfile_wp << it->first <<" ; "<< it->second << endl;
	}
	csvfile_wp.close();

	cout<<"Exit"<<endl;
	return EXIT_SUCCESS;
}
