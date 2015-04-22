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
#include <ctime>
#include "pugixml.hpp"
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/tuple/tuple.hpp"
#include "sys/times.h"
#include "sys/vtimes.h"

using namespace std;
using namespace boost;

map<string,map<string, int> > stl_wordCount,stl_tagCount;
map<string,map< int, int> > stl_postTime,stl_postAnswer,stl_postScore,stl_userReputation,stl_userAboutMeWords,stl_userAge;
map<string,map< int, vector<string> > > stl_userBadges;
map<string,map< int, int[2] > > stl_PostVotes;

typedef unordered_map<string,unordered_map<string, int> > umap_1;
umap_1 b_wordCount,b_tagCount;
typedef unordered_map<string,unordered_map< int, int> > umap_2;
umap_2 b_postTime,b_postAnswer,b_postScore,b_userReputation,b_userAboutMeWords,b_userAge;
typedef unordered_map<string,unordered_map< int, vector<string> > > umap_3;
umap_3 b_userBadges;
typedef unordered_map<string,unordered_map< int, boost::array<int, 2>> > umap_4;
umap_4 b_PostVotes;

pugi::xml_parse_result result;
pugi::xml_node node;
pugi::xml_document doc;

struct letter_only: std::ctype<char>
{
    letter_only(): std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table()
    {
        static std::vector<std::ctype_base::mask> rc(std::ctype<char>::table_size,std::ctype_base::space);
        std::fill(&rc['A'], &rc['z'+1], std::ctype_base::alpha);
        return &rc[0];
    }
};

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

double getCPUUsage(){
	long double a[4], b[4], loadavg;
	FILE *fp;
	for(;;)
	{
		fp = fopen("/proc/stat","r");
		int i=fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
		fclose(fp);
		sleep(1);

		fp = fopen("/proc/stat","r");
		i=fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
		fclose(fp);
		loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
		return loadavg;
	}
}

int parseLine(char* line)
{
	int i = strlen(line);
	while (*line < '0' || *line > '9') line++;
	line[i-3] = '\0';
	i = atoi(line);
	return i;
}

int getVMValue(){ //Note: this value is in KB!
	FILE* file = fopen("/proc/self/status", "r");
	int result = -1;
	char line[128];
	while (fgets(line, 128, file) != NULL)
	{
		if (strncmp(line, "VmSize:", 7) == 0)
		{
			result = parseLine(line);
			break;
		}
	}
	fclose(file);
	return result;
}

int getRAMValue(){ //Note: this value is in KB!
	FILE* file = fopen("/proc/self/status", "r");
	int result = -1;
	char line[128];

	while (fgets(line, 128, file) != NULL)
	{
		if (strncmp(line, "VmRSS:", 6) == 0)
		{
			result = parseLine(line);
			break;
		}
	}
	fclose(file);
	return result;
}

void loadPosts(string site)
{
	string filename="./"+site+"/Posts.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
	    cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
	    cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("posts");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int postId;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string name = ait->name();
			if(name=="Id")
			{
				postId=boost::lexical_cast<int>(ait->value());
			}
			if(name=="AcceptedAnswerId")
			{
				int answer=boost::lexical_cast<int>(ait->value());
				stl_postAnswer[site][postId]=answer;
			}
			if(name=="Body")
			{
				stringstream ss;
				ss.imbue(std::locale(std::locale(), new letter_only())); //enable reading only letters!
				ss << ait->value();
				string word;
				while(ss >> word)
				{
					++stl_wordCount[site][word];
				}
			}
			if(name=="Score")
			{
				stl_postScore[site][postId]=boost::lexical_cast<int>(ait->value());
			}
			if(name=="CreationDate")
			{
				string datetime = ait->value();
				vector<string> x=split(datetime,'T');
				vector<string> time= split(x[1],':');
				++stl_postTime[site][boost::lexical_cast<int>(time[0])];
			}
		}
	}
}
void loadPosts_boost(string site)
{
	string filename="./"+site+"/Posts.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
	    cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
	    cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("posts");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int postId;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string name = ait->name();
			if(name=="Id")
			{
				postId=boost::lexical_cast<int>(ait->value());
			}
			if(name=="AcceptedAnswerId")
			{
				int answer=boost::lexical_cast<int>(ait->value());
				b_postAnswer[site][postId]=answer;
			}
			if(name=="Body")
			{
				stringstream ss;
				ss.imbue(std::locale(std::locale(), new letter_only())); //enable reading only letters!
				ss << ait->value();
				string word;
				while(ss >> word)
				{
					++b_wordCount[site][word];
				}
			}
			if(name=="Score")
			{
				b_postScore[site][postId]=boost::lexical_cast<int>(ait->value());
			}
			if(name=="CreationDate")
			{
				string datetime = ait->value();
				vector<string> x=split(datetime,'T');
				vector<string> time= split(x[1],':');
				++b_postTime[site][boost::lexical_cast<int>(time[0])];
			}
		}
	}
}

void loadPostsHistory(string site)
{
	string filename="./"+site+"/PostHistory.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("posthistory");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
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
					++stl_wordCount[site][word];
				}
			}
			if(name=="CreationDate")
			{
				string datetime = ait->value();
				vector<string> x=split(datetime,'T');
				vector<string> time= split(x[1],':');
				++stl_postTime[site][boost::lexical_cast<int>(time[0])];
			}
		}
	}
}
void loadPostsHistory_boost(string site)
{
	string filename="./"+site+"/PostHistory.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("posthistory");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
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
					++b_wordCount[site][word];
				}
			}
			if(name=="CreationDate")
			{
				string datetime = ait->value();
				vector<string> x=split(datetime,'T');
				vector<string> time= split(x[1],':');
				++b_postTime[site][boost::lexical_cast<int>(time[0])];
			}
		}
	}
}

void loadComments(string site)
{
	string filename="./"+site+"/Comments.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}

	node = doc.child("comments");

	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
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
					++stl_wordCount[site][word];
				}
			}
			if(name=="CreationDate")
			{
				string datetime = ait->value();
				vector<string> x=split(datetime,'T');
				vector<string> time= split(x[1],':');
				++stl_postTime[site][boost::lexical_cast<int>(time[0])];
			}
		}
	}
}
void loadComments_boost(string site)
{
	string filename="./"+site+"/Comments.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("comments");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
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
					++b_wordCount[site][word];
				}
			}
			if(name=="CreationDate")
			{
				string datetime = ait->value();
				vector<string> x=split(datetime,'T');
				vector<string> time= split(x[1],':');
				++b_postTime[site][boost::lexical_cast<int>(time[0])];
			}
		}
	}
}

void loadUsers (string site)
{
	string filename="./"+site+"/Users.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("users");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int id,reputation,aboutWordsCount;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string sname = ait->name();
			if(sname=="Id")
			{
				id=boost::lexical_cast<int>(ait->value());
			}
			if(sname=="Reputation")
			{
				reputation=boost::lexical_cast<int>(ait->value());
				stl_userReputation[site][id]=reputation;
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
				stl_userAboutMeWords[site][id]=aboutWordsCount;
			}
			if(sname=="Age")
			{
				int age=boost::lexical_cast<int>(ait->value());
				stl_userAge[site][id]=age;
			}
		}
	}
}
void loadUsers_boost(string site)
{
	string filename="./"+site+"/Users.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("users");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int id,reputation,aboutWordsCount;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string sname = ait->name();
			if(sname=="Id")
			{
				id=boost::lexical_cast<int>(ait->value());
			}
			if(sname=="Reputation")
			{
				reputation=boost::lexical_cast<int>(ait->value());
				b_userReputation[site][id]=reputation;
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
				b_userAboutMeWords[site][id]=aboutWordsCount;
			}
		}
	}
}

void loadTags(string site)
{
	string filename = "./"+site+"/Tags.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("tags");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
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
				int value = boost::lexical_cast<int>( svalue );
				stl_tagCount[site][name]=value;
			}
		}
	}
}
void loadTags_boost(string site)
{
	string filename = "./"+site+"/Tags.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("tags");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
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
				int value = boost::lexical_cast<int>( svalue );
				b_tagCount[site][name]=value;
			}
		}
	}
}

void loadBadges(string site)
{
	string filename="./"+site+ "/Badges.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("badges");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int id;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string sname = ait->name();
			if(sname=="UserId")
			{
				id=boost::lexical_cast<int>(ait->value());
			}
			if(sname=="Name")
			{
				string badge=ait->value();
				stl_userBadges[site][id].push_back (badge);
			}
		}
	}
}
void loadBadges_boost(string site)
{
	string filename="./"+site+ "/Badges.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("badges");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int id=0;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string sname = ait->name();
			if(sname=="UserId")
			{
				id=boost::lexical_cast<int>(ait->value());
			}
			if(sname=="Name")
			{
				string badge=ait->value();;
				b_userBadges[site][id].push_back (badge);
			}
		}
	}
}

void loadVotes(string site)
{
	string filename = "./"+site+"/Votes.xml";
	result = doc.load_file(filename.c_str());
	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("votes");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int id;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string sname = ait->name();
			if(sname=="PostId")
			{
				id=boost::lexical_cast<int>(ait->value());
			}
			if(sname=="VoteTypeId")
			{
				int vote=boost::lexical_cast<int>(ait->value());
				if(vote==2)
				{
					++stl_PostVotes[site][id][0];
				}
				if(vote==3)
				{
					++stl_PostVotes[site][id][1];
				}

			}
		}
	}
}
void loadVotes_boost(string site)
{
	string filename = "./"+site+"/Votes.xml";
	result = doc.load_file(filename.c_str());

	if(!result)
	{
		cout << "XML [" << filename << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		cout << "Error description: " << result.description() << "\n";
	}
	node = doc.child("votes");
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int id;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string sname = ait->name();
			if(sname=="PostId")
			{
				id=boost::lexical_cast<int>(ait->value());
			}
			if(sname=="VoteTypeId")
			{
				int vote=boost::lexical_cast<int>(ait->value());
				if(vote==2)
				{
					++b_PostVotes[site][id][0];
				}
				if(vote==3)
				{
					++b_PostVotes[site][id][1];
				}
			}
		}
	}
}

void writeWordFrequency(string site)
{
	string filename="WordFrequency_"+site+".csv";
	ofstream csvfile_freq (filename.c_str());
	csvfile_freq << "Word;Frequency" << endl;
	for (std::map<std::string, int>::iterator it = stl_wordCount[site].begin(); it != stl_wordCount[site].end(); ++it)
	{
		csvfile_freq << it->first <<";"<< it->second << endl;
	}
	csvfile_freq.close();
}
void writeWordFrequency_boost(string site)
{
	string filename="WordFrequency_"+site+"_boost.csv";
	ofstream csvfile_freq (filename.c_str());
	csvfile_freq << "Word;Frequency" << endl;
	for (auto it = b_wordCount[site].begin(); it != b_wordCount[site].end(); ++it)
	{
			csvfile_freq << it->first <<";"<< it->second << endl;
	}
	csvfile_freq.close();
}

void writePostTime(string site)
{
	string filename="PostTime_"+site+".csv";
	ofstream csvfile (filename.c_str());
	csvfile << "Hour;Posts" << endl;
	for (auto it = stl_postTime[site].begin(); it != stl_postTime[site].end(); ++it)
	{
		csvfile << it->first <<";"<< it->second << endl;
	}
	csvfile.close();
}
void writePostTime_boost(string site)
{
	string filename="PostTime_"+site+"_boost.csv";
	ofstream csvfile (filename.c_str());
	csvfile << "Hour;Posts" << endl;
	for (auto it = b_postTime[site].begin(); it != b_postTime[site].end(); ++it)
	{
		csvfile << it->first <<";"<< it->second << endl;
	}
	csvfile.close();
}

void writeTagFrequency(string site)
{
	string filename="TagFrequency_"+site+".csv";
	ofstream csvfile_tagfreq (filename.c_str());
	csvfile_tagfreq << "Tag;Frequency" << endl;
	for (auto it = stl_tagCount[site].begin(); it != stl_tagCount[site].end(); ++it)
	{
		csvfile_tagfreq << it->first <<";"<< it->second << endl;
	}
	csvfile_tagfreq.close();
}
void writeTagFrequency_boost(string site)
{
	string filename="TagFrequency_"+site+"_boost.csv";
	ofstream csvfile_tagfreq (filename.c_str());
	csvfile_tagfreq << "Tag;Frequency" << endl;
	for (auto it = b_tagCount[site].begin(); it != b_tagCount[site].end(); ++it)
	{
		csvfile_tagfreq << it->first <<";"<< it->second << endl;
	}
	csvfile_tagfreq.close();
}

void writePostScore(string site)
{
	string filename="PostScore_"+site+".csv";
	ofstream csvfile_postScore (filename.c_str());
	csvfile_postScore << "Post;Score" << endl;
	for (std::map<int , int>::iterator it = stl_postScore[site].begin(); it != stl_postScore[site].end(); ++it)
	{
		csvfile_postScore << it->first <<";"<< it->second << endl;
	}
	csvfile_postScore.close();
}
void writePostScore_boost(string site)
{
	string filename="PostScore_"+site+"_boost.csv";
	ofstream csvfile_postScore (filename.c_str());
	csvfile_postScore << "Post;Score" << endl;
	for (auto it = b_postScore[site].begin(); it != b_postScore[site].end(); ++it)
	{
		csvfile_postScore << it->first <<";"<< it->second << endl;
	}
	csvfile_postScore.close();
}

void writeUserAge(string site)
{
	string filename="UserAge_"+site+".csv";
	ofstream csvfile_postScore (filename.c_str());
	csvfile_postScore << "User;Age " << endl;
	for (std::map<int , int>::iterator it = stl_postScore[site].begin(); it != stl_postScore[site].end(); ++it)
	{
		csvfile_postScore << it->first <<";"<< it->second << endl;
	}
	csvfile_postScore.close();
}
void writeUserAge_boost(string site)
{
	string filename="UserAge_"+site+"_boost.csv";
	ofstream csvfile_postScore (filename.c_str());
	csvfile_postScore << "User;Age " << endl;
	for (auto it = b_postScore[site].begin(); it != b_postScore[site].end(); ++it)
	{
		csvfile_postScore << it->first <<";"<< it->second << endl;
	}
	csvfile_postScore.close();
}

void writePostAnswer(string site)
{
	string filename="PostAnswer_"+site+".csv";
	ofstream csvfile_postAnswer (filename.c_str());
	csvfile_postAnswer << "Post;Answer" << endl;
	for (std::map<int , int>::iterator it = stl_postAnswer[site].begin(); it != stl_postAnswer[site].end(); ++it)
	{
		csvfile_postAnswer << it->first <<";"<< it->second << endl;
	}
	csvfile_postAnswer.close();
}
void writePostAnswer_boost(string site)
{
	string filename="PostAnswer_"+site+"_boost.csv";
	ofstream csvfile_postAnswer (filename.c_str());
	csvfile_postAnswer << "Post;Answer" << endl;
	for (auto it = b_postAnswer[site].begin(); it != b_postAnswer[site].end(); ++it)
	{
		csvfile_postAnswer << it->first <<";"<< it->second << endl;
	}
	csvfile_postAnswer.close();
}

void writeUserAboutMeWords(string site)
{
	string filename="UserAboutMeWords_"+site+".csv";
	ofstream csvfile_userAboutMeWords_sf (filename.c_str());
	csvfile_userAboutMeWords_sf << "User;WordCountAboutMe " << endl;
	for (std::map<int , int>::iterator it = stl_userAboutMeWords[site].begin(); it != stl_userAboutMeWords[site].end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<";"<< it->second << endl;
	}
	csvfile_userAboutMeWords_sf.close();
}
void writeUserAboutMeWords_boost(string site)
{
	string filename="UserAboutMeWords_"+site+"_boost.csv";
	ofstream csvfile_userAboutMeWords_sf (filename.c_str());
	csvfile_userAboutMeWords_sf << "User;WordCountAboutMe " << endl;
	for (auto it = b_userAboutMeWords[site].begin(); it != b_userAboutMeWords[site].end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<";"<< it->second << endl;
	}
	csvfile_userAboutMeWords_sf.close();
}

void writePostVotes(string site)
{
	string filename="PostVotes_"+site+".csv";
	ofstream csvfile_userAboutMeWords_sf (filename.c_str());
	csvfile_userAboutMeWords_sf << "Post;Up;Down;Total" << endl;
	for (auto it = stl_PostVotes[site].begin(); it != stl_PostVotes[site].end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<";"<< it->second[0] <<";"<< it->second[1]<<";"<< it->second[0]-it->second[1] << endl;
	}
	csvfile_userAboutMeWords_sf.close();
}
void writePostVotes_boost(string site)
{
	string filename="PostVotes_"+site+"_boost.csv";
	ofstream csvfile_userAboutMeWords_sf (filename.c_str());
	csvfile_userAboutMeWords_sf << "Post;Up;Down;Total" << endl;
	for (auto it = stl_PostVotes[site].begin(); it != stl_PostVotes[site].end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<";"<< it->second[0] <<";"<< it->second[1]<<";"<< it->second[0]-it->second[1] << endl;
	}
	csvfile_userAboutMeWords_sf.close();
}

void writeUserReputation(string site)
{
	string filename="UserReputation_"+site+".csv";
	ofstream csvfile_userReputation (filename.c_str());
	csvfile_userReputation << "User;Reputation" << endl;
	for (std::map<int , int>::iterator it = stl_userReputation[site].begin(); it != stl_userReputation[site].end(); ++it)
	{
		csvfile_userReputation << it->first <<";"<< it->second << endl;
	}
	csvfile_userReputation.close();
}
void writeUserReputation_boost(string site)
{
	string filename="UserReputation_"+site+"_boost.csv";
	ofstream csvfile_userReputation (filename.c_str());
	csvfile_userReputation << "User;Reputation" << endl;
	for (auto it = b_userReputation[site].begin(); it != b_userReputation[site].end(); ++it)
	{
		csvfile_userReputation << it->first <<";"<< it->second << endl;
	}
	csvfile_userReputation.close();
}

void writeBadges(string site)
{
	string filename1="UserBadges_"+site+".csv";
	ofstream csvfile_userBadges_sf (filename1.c_str());
	string filename2="UserBadgeCount_"+site+".csv";
	ofstream csvfile_userBadgeCount_sf (filename2.c_str());
	csvfile_userBadges_sf << "User;Badge" << endl;
	csvfile_userBadgeCount_sf << "User;BadgeCount" << endl;
	for (auto it = stl_userBadges[site].begin(); it != stl_userBadges[site].end(); ++it)
	{
		vector<string> badges=it->second;
		csvfile_userBadgeCount_sf<< it->first <<";"<< badges.size() << endl;
		for (auto it2 = badges.begin(); it2 != badges.end(); ++it2)
		{
			csvfile_userBadges_sf << it->first <<";"<< *it2 << endl;
		}
	}
	csvfile_userBadges_sf.close();
	csvfile_userBadgeCount_sf.close();
}
void writeBadges_boost(string site)
{
	string filename1="UserBadges_"+site+"_boost.csv";
	ofstream csvfile_userBadges (filename1.c_str());
	string filename2="UserBadgeCount_"+site+"_boost.csv";
	ofstream csvfile_userBadgeCount (filename2.c_str());
	csvfile_userBadges << "User;Badge" << endl;
	csvfile_userBadgeCount << "User;BadgeCount" << endl;
	for (auto it = b_userBadges[site].begin(); it != b_userBadges[site].end(); ++it)
	{
		vector<string> badges=it->second;
		csvfile_userBadgeCount<< it->first <<";"<< badges.size() << endl;
		for (auto it2 = badges.begin(); it2 != badges.end(); ++it2)
		{
			csvfile_userBadges << it->first <<";"<< *it2 << endl;
		}

	}
	csvfile_userBadges.close();
	csvfile_userBadgeCount.close();
}

int main( int argc, char ** argv )
{
	time_t rawtime;

	time (&rawtime);

	string filename="Log "+string(ctime (&rawtime))+".log";
	string filenamet="Log "+string(ctime (&rawtime))+"_t.log";
	ofstream log (filename.c_str());
	ofstream logtime (filenamet.c_str());
	log<<"Site;Container;Virtual Mem [kB];RAM [kB];CPU [%]"<<endl;
	logtime	<<"Site;Container;Time [s]"<<endl;
	clock_t begin = clock();

	string fromSite="serverfault";
	loadPosts(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadPostsHistory(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadComments(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadUsers(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadTags(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadBadges(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadVotes(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;

	writeWordFrequency(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeTagFrequency(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserAge(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostScore(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostTime(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostAnswer(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserReputation(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserAboutMeWords(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeBadges(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostVotes(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	/******* FIN SF ****/
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	logtime << "STL;"<<fromSite<<";"<<elapsed_secs<< endl;

	begin = clock();
	loadPosts_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadPostsHistory_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadComments_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadUsers_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadTags_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadBadges_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadVotes_boost(fromSite);

	writeWordFrequency_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeTagFrequency_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserAge_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostScore_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostTime_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostAnswer_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserReputation_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserAboutMeWords_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeBadges_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostVotes_boost(fromSite);

	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	logtime << "Boost;"<<fromSite<<";"<<elapsed_secs<< endl;

	//Cambio sitio

	begin = clock();
	fromSite="wordpress";
	loadPosts(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadPostsHistory(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadComments(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadUsers(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadTags(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadBadges(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadVotes(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;

	writeWordFrequency(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeTagFrequency(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserAge(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostScore(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostTime(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostAnswer(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserReputation(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserAboutMeWords(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeBadges(fromSite);
	log<<fromSite<<";"<<"STL"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostVotes(fromSite);

	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	logtime << "STL;"<<fromSite<<";"<<elapsed_secs<<endl;

	begin = clock();
	loadPosts_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadPostsHistory_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadComments_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadUsers_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadTags_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadBadges_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	loadVotes_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;

	writeWordFrequency_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeTagFrequency_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserAge_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostScore_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostTime_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostAnswer_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserReputation_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeUserAboutMeWords_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writeBadges_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	writePostVotes_boost(fromSite);
	log<<fromSite<<";"<<"Boost"<<";"<<getVMValue()<<";"<<getRAMValue()<<";"<<getCPUUsage()<<endl;
	/***** fin WP   */
	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	logtime << "Boost;"<<fromSite<<";"<<elapsed_secs << endl;

	log.close();
	logtime.close();
	return EXIT_SUCCESS;
}
