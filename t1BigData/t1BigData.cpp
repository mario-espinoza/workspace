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
#include  <boost/unordered_map.hpp>
/*
#include <boost/property_tree/xml_parser.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/xml_iarchive.hpp>
*/

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

int main( int argc, char ** argv )
{
	std::map<std::string, int> stl_wordCount_sf,stl_wordCount_wp,stl_tagCount_sf;
	typedef boost::unordered_map<std::string, int> b_wordCount_sf;

	pugi::xml_document doc_posts_sf,doc_posts_wp;

	cout<<"Posts sf"<<endl;
	pugi::xml_parse_result result_posts_sf = doc_posts_sf.load_file("./serverfault/Posts.xml");
	pugi::xml_node node_sf = doc_posts_sf.child("posts");

	for (pugi::xml_node_iterator it = node_sf.begin(); it != node_sf.end(); ++it)
	{
		int postId;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{

			string name = ait->name();
			if(name=="Body")
			{

			}
			if(name=="Body")
			{
				stringstream ss;

				ss.imbue(std::locale(std::locale(), new letter_only())); //enable reading only letters!

				ss << ait->value();
				string word;
				while(ss >> word)
				{
					++stl_wordCount_sf[word];
				}
			}
			if(name=="")
			{

			}
		}
	}

	result_posts_sf = doc_posts_sf.load_file("./serverfault/PostsHistory.xml");
	node_sf = doc_posts_sf.child("posthistory");
	cout<<"History SF"<<endl;
	for (pugi::xml_node_iterator it = node_sf.begin(); it != node_sf.end(); ++it)
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
					++stl_wordCount_sf[word];
				}
			}
		}
	}

	result_posts_sf = doc_posts_sf.load_file("./serverfault/Comments.xml");
	node_sf = doc_posts_sf.child("comments");
	cout<<"Comments SF"<<endl;
	for (pugi::xml_node_iterator it = node_sf.begin(); it != node_sf.end(); ++it)
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
					++stl_wordCount_sf[word];
				}
			}
		}
	}

	result_posts_sf = doc_posts_sf.load_file("./serverfault/Tags.xml");
		node_sf = doc_posts_sf.child("tags");
		cout<<"Tags SF"<<endl;
		for (pugi::xml_node_iterator it = node_sf.begin(); it != node_sf.end(); ++it)
		{
			for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
			{
				string sname = ait->name();
				if(sname=="TagName")
				{
					string name=ait->value();
					string count;
					do {
						ait++;
						count = ait->name();
					} while (count!="Count");
					string svalue=ait->value();
					int value = stoi( svalue );
					//cout<<name<<" "<<value<<endl;

					stl_tagCount_sf[name]=value;
				}
			}
		}


	/******* FIN SF ****/

	/****** WP */

	pugi::xml_parse_result result_posts_wp = doc_posts_wp.load_file("./wordpress/Posts.xml");
	pugi::xml_node posts_node_wp = doc_posts_wp.child("posts");


	/***** fin WP   */
	cout<<"SF"<<endl;
	ofstream csvfile_freq_sf ("WordFrequency_SF.csv");
	csvfile_freq_sf << "Word; Frequency " << endl;
	for (std::map<std::string, int>::iterator it = stl_wordCount_sf.begin(); it != stl_wordCount_sf.end(); ++it)
	{
		csvfile_freq_sf << it->first <<" ; "<< it->second << endl;
	}
	csvfile_freq_sf.close();

	ofstream csvfile_tagfreq_sf ("TagFrequency_SF.csv");
	csvfile_tagfreq_sf << "Tag; Frequency " << endl;
	for (std::map<std::string, int>::iterator it = stl_tagCount_sf.begin(); it != stl_tagCount_sf.end(); ++it)
	{
		csvfile_tagfreq_sf << it->first <<" ; "<< it->second << endl;
	}
	csvfile_tagfreq_sf.close();



	cout<<"WP"<<endl;
	ofstream csvfile_freq_wp ("WordFrequency_WP.csv");
	csvfile_freq_wp << "Word; Frequency " << endl;
	for (std::map<std::string, int>::iterator it = stl_wordCount_wp.begin(); it != stl_wordCount_wp.end(); ++it)
	{
		csvfile_freq_wp << it->first <<" ; "<< it->second << endl;
	}
	csvfile_freq_wp.close();

	cout<<"Exit"<<endl;
	return EXIT_SUCCESS;
}
