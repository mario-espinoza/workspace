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
	std::map< int, int> stl_postScore_sf;
	std::map<int,int> stl_userReputation_sf,stl_userAboutMeWords_sf;
	std::map<int, vector<string> > stl_userBadges_sf;
	typedef boost::unordered_map<std::string, int> b_wordCount_sf;

	pugi::xml_document doc_sf,doc_wp;

	cout<<"Posts sf"<<endl;
	pugi::xml_parse_result result_sf = doc_sf.load_file("./serverfault/Posts.xml");
	pugi::xml_node node_sf = doc_sf.child("posts");

	for (pugi::xml_node_iterator it = node_sf.begin(); it != node_sf.end(); ++it)
	{
		int postId;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{

			string name = ait->name();
			if(name=="AcceptedAnswerId")
			{
				int answer=stoi(ait->value());
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
			if(name=="Score")
			{
				cout<<postId<<" "<<ait->value()<<endl;
				stl_postScore_sf[postId]=stoi(ait->value());
			}
		}
	}



	result_sf = doc_sf.load_file("./serverfault/PostsHistory.xml");
	node_sf = doc_sf.child("posthistory");
	cout<<"Post History SF"<<endl;
	for (pugi::xml_node_iterator it = node_sf.begin(); it != node_sf.end(); ++it)
	{
		int postId;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string name = ait->name();
			if(name=="Id")
			{
				postId=stoi(ait->value());
			}
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

	result_sf = doc_sf.load_file("./serverfault/Comments.xml");
	node_sf = doc_sf.child("comments");
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

	result_sf = doc_sf.load_file("./serverfault/Users.xml");
	node_sf = doc_sf.child("users");
	cout<<"Users SF"<<endl;
	for (pugi::xml_node_iterator it = node_sf.begin(); it != node_sf.end(); ++it)
	{
		int id,reputation,aboutWordsCount;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string sname = ait->name();

			if(sname=="Id")
			{
				id=stoi(ait->value());
			}
			if(sname=="Reputation")
			{
				reputation=stoi(ait->value());

				stl_userReputation_sf[id]=reputation;
			}
			if(sname=="AboutMe")
			{
				aboutWordsCount=0;
				stringstream ss;

				ss.imbue(std::locale(std::locale(), new letter_only())); //enable reading only letters!

				ss << ait->value();
				string word;
				while(ss >> word)
				{
					++aboutWordsCount;
				}

				stl_userAboutMeWords_sf[id]=aboutWordsCount;
			}
		}
	}

	result_sf = doc_sf.load_file("./serverfault/Tags.xml");
	node_sf = doc_sf.child("tags");
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

				stl_tagCount_sf[name]=value;
			}
		}
	}

	result_sf = doc_sf.load_file("./serverfault/Badges.xml");
	node_sf = doc_sf.child("badges");
	cout<<"Badges SF"<<endl;
	for (pugi::xml_node_iterator it = node_sf.begin(); it != node_sf.end(); ++it)
	{
		int id;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string sname = ait->name();
			if(sname=="UserId")
			{
				id=stoi(ait->value());
			}
			if(sname=="Name")
			{
				string badge=ait->value();;
				stl_userBadges_sf[id].push_back (badge);
			}
		}
	}

	/******* FIN SF ****/

	/****** WP */

	pugi::xml_parse_result result_posts_wp = doc_wp.load_file("./wordpress/Posts.xml");
	pugi::xml_node posts_node_wp = doc_wp.child("posts");


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

	ofstream csvfile_postScore_sf ("PostScore_SF.csv");
	csvfile_postScore_sf << "Post;Score " << endl;
	for (std::map<int , int>::iterator it = stl_postScore_sf.begin(); it != stl_postScore_sf.end(); ++it)
	{
		csvfile_postScore_sf << it->first <<" ; "<< it->second << endl;
	}
	csvfile_postScore_sf.close();

	ofstream csvfile_userReputation_sf ("UserReputation_SF.csv");
	csvfile_userReputation_sf << "User;Reputation " << endl;
	for (std::map<int , int>::iterator it = stl_userReputation_sf.begin(); it != stl_userReputation_sf.end(); ++it)
	{
		csvfile_userReputation_sf << it->first <<" ; "<< it->second << endl;
	}
	csvfile_userReputation_sf.close();

	ofstream csvfile_userAboutMeWords_sf ("UserAboutMeWords_SF.csv");
	csvfile_userAboutMeWords_sf << "User;WordCountAboutMe " << endl;
	for (std::map<int , int>::iterator it = stl_userAboutMeWords_sf.begin(); it != stl_userAboutMeWords_sf.end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<" ; "<< it->second << endl;
	}
	csvfile_userAboutMeWords_sf.close();

	ofstream csvfile_userBadges_sf ("UserBadges_SF.csv");
	ofstream csvfile_userBadgeCount_sf ("UserBadgeCount_SF.csv");
	csvfile_userBadges_sf << "User;Badge " << endl;
	for (std::map<int , vector<string>>::iterator it = stl_userBadges_sf.begin(); it != stl_userBadges_sf.end(); ++it)
	{
		vector<string> badges=it->second;
		csvfile_userBadgeCount_sf<< it->first <<" ; "<< badges.size() << endl;
		for (vector<string>::iterator it2 = badges.begin(); it2 != badges.end(); ++it2)
		{
			csvfile_userBadges_sf << it->first <<" ; "<< *it2 << endl;
		}

	}
	csvfile_userBadges_sf.close();
	csvfile_userBadgeCount_sf.close();

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
