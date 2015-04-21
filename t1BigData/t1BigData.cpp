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

static const int MAX = 3;
static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;

map<string,map<string, int> > stl_wordCount,stl_tagCount;
map<string,map< int, int> > stl_postAnswer,stl_postScore,stl_userReputation,stl_userAboutMeWords;
map<string,map< int, vector<string> > > stl_userBadges;
map<string,map< int, int[3] > > stl_PostVotes;

typedef unordered_map<string,unordered_map<string, int> > umap_1;
umap_1 b_wordCount,b_tagCount;
typedef unordered_map<string,unordered_map< int, int> > umap_2;
umap_2 b_postAnswer,b_postScore,b_userReputation,b_userAboutMeWords;
typedef unordered_map<string,unordered_map< int, vector<string> > > umap_3;
umap_3 b_userBadges;
typedef unordered_map<string,unordered_map< int, boost::array<int, 3>> > umap_4;
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

void init(){
    FILE* file;
    struct tms timeSample;
    char line[128];

    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    file = fopen("/proc/cpuinfo", "r");
    numProcessors = 0;
    while(fgets(line, 128, file) != NULL){
        if (strncmp(line, "processor", 9) == 0) numProcessors++;
    }
    fclose(file);
}


double getCurrentCPUValue(){
    struct tms timeSample;
    clock_t now;
    double percent;


    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
        timeSample.tms_utime < lastUserCPU){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        percent = (timeSample.tms_stime - lastSysCPU) +
            (timeSample.tms_utime - lastUserCPU);
        percent /= (now - lastCPU);
        percent /= numProcessors;
        percent *= 100;
    }
    lastCPU = now;
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    return percent;
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
	int contador=0;
	cout<<"Posts "<<site<<endl;
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
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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
	cout<<"Post History "<<site<<endl;

	int contador=0;
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
	{
		int postId=0;
		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			string name = ait->name();
			if(name=="Id")
			{
				postId=boost::lexical_cast<int>(ait->value());
			}
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
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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
	cout<<"Comments "<<site<<endl;

	int contador=0;
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
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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

	int contador=0;
	node = doc.child("users");
	cout<<"Users "<<site<<endl;
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
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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
	int contador=0;
	node = doc.child("tags");
	cout<<"Tags "<<site<<endl;
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
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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

	node = doc.child("badges");int contador=0;
	cout<<"Badges "<<site<<endl;

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
				string badge=ait->value();;

				stl_userBadges[site][id].push_back (badge);
			}
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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
	//int contador=0;
	node = doc.child("votes");
	cout<<"Votes "<<site<<endl;
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
					++stl_PostVotes[site][id][2];
				}
				if(vote==3)
				{
					++stl_PostVotes[site][id][1];
					--stl_PostVotes[site][id][2];
				}

			}
		}
	}
}

void loadPosts_boost(string site)
{
	int contador=0;
	cout<<"Posts "<<site<<endl;
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
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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
	cout<<"Post History "<<site<<endl;

	int contador=0;
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
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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
	cout<<"Comments "<<site<<endl;

	int contador=0;
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
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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

	int contador=0;
	node = doc.child("users");
	cout<<"Users "<<site<<endl;
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
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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
	int contador=0;
	node = doc.child("tags");
	cout<<"Tags "<<site<<endl;
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
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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

	node = doc.child("badges");int contador=0;
	cout<<"Badges "<<site<<endl;

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
				string badge=ait->value();;

				stl_userBadges[site][id].push_back (badge);
			}
		}
		contador++;
	}
	cout<< contador<<" "<<site<<endl;
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
	//int contador=0;
	node = doc.child("votes");
	cout<<"Votes "<<site<<endl;
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
					++b_PostVotes[site][id][2];
				}
				if(vote==3)
				{
					++b_PostVotes[site][id][1];
					--b_PostVotes[site][id][2];
				}

			}
		}
	}
}


void writeWordFrequency(string site)
{
	cout<<"SF"<<endl;
	string filename="WordFrequency_"+site+".csv";
	ofstream csvfile_freq (filename.c_str());
	csvfile_freq << "Word; Frequency " << endl;
	for (std::map<std::string, int>::iterator it = stl_wordCount[site].begin(); it != stl_wordCount[site].end(); ++it)
	{
		csvfile_freq << it->first <<" ; "<< it->second << endl;
	}
	csvfile_freq.close();
}

void writeTagFrequency(string site)
{
	string filename="TagFrequency_"+site+".csv";
	ofstream csvfile_tagfreq (filename.c_str());
	csvfile_tagfreq << "Tag; Frequency " << endl;
	for (std::map<std::string, int>::iterator it = stl_tagCount[site].begin(); it != stl_tagCount[site].end(); ++it)
	{
		csvfile_tagfreq << it->first <<" ; "<< it->second << endl;
	}
	csvfile_tagfreq.close();
}

void writePostScore(string site)
{
	string filename="PostScore_"+site+".csv";
	ofstream csvfile_postScore (filename.c_str());
	csvfile_postScore << "Post;Score " << endl;
	for (std::map<int , int>::iterator it = stl_postScore[site].begin(); it != stl_postScore[site].end(); ++it)
	{
		csvfile_postScore << it->first <<" ; "<< it->second << endl;
	}
	csvfile_postScore.close();
}

void writePostAnswer(string site)
{
	string filename="PostAnswer_"+site+".csv";
	ofstream csvfile_postAnswer (filename.c_str());
	csvfile_postAnswer << "Post;Answer " << endl;
	for (std::map<int , int>::iterator it = stl_postAnswer[site].begin(); it != stl_postAnswer[site].end(); ++it)
	{
		csvfile_postAnswer << it->first <<" ; "<< it->second << endl;
	}
	csvfile_postAnswer.close();
}

void writeUserReputation(string site)
{
	string filename="UserReputation_"+site+".csv";
	ofstream csvfile_userReputation (filename.c_str());
	csvfile_userReputation << "User;Reputation " << endl;
	for (std::map<int , int>::iterator it = stl_userReputation[site].begin(); it != stl_userReputation[site].end(); ++it)
	{
		csvfile_userReputation << it->first <<" ; "<< it->second << endl;
	}
	csvfile_userReputation.close();
}

void writeUserAboutMeWords(string site)
{
	string filename="UserAboutMeWords_"+site+".csv";
	ofstream csvfile_userAboutMeWords_sf (filename.c_str());
	csvfile_userAboutMeWords_sf << "User;WordCountAboutMe " << endl;
	for (std::map<int , int>::iterator it = stl_userAboutMeWords[site].begin(); it != stl_userAboutMeWords[site].end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<" ; "<< it->second << endl;
	}
	csvfile_userAboutMeWords_sf.close();
}

void writePostVotes(string site)
{
	string filename="PostVotes_"+site+".csv";
	ofstream csvfile_userAboutMeWords_sf (filename.c_str());
	csvfile_userAboutMeWords_sf << "Post;Up;Down;Total " << endl;
	for (std::map<int , int[3]>::iterator it = stl_PostVotes[site].begin(); it != stl_PostVotes[site].end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<" ; "<< it->second[0] <<" ; "<< it->second[1]<<" ; "<< it->second[2] << endl;
	}
	csvfile_userAboutMeWords_sf.close();
}

void writeBadges(string site)
{
	string filename1="UserBadges_"+site+".csv";
	ofstream csvfile_userBadges_sf (filename1.c_str());
	string filename2="UserBadgeCount_"+site+".csv";
	ofstream csvfile_userBadgeCount_sf (filename2.c_str());
	csvfile_userBadges_sf << "User;Badge " << endl;
	for (std::map<int , vector<string>>::iterator it = stl_userBadges[site].begin(); it != stl_userBadges[site].end(); ++it)
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
}

void writeWordFrequency_boost(string site)
{
	string filename="WordFrequency_"+site+"_boost.csv";
	ofstream csvfile_freq (filename.c_str());
	csvfile_freq << "Word; Frequency " << endl;
	for (auto it = b_wordCount[site].begin(); it != b_wordCount[site].end(); ++it)
	{
			csvfile_freq << it->first <<" ; "<< it->second << endl;
	}
	csvfile_freq.close();
}

void writeTagFrequency_boost(string site)
{
	string filename="TagFrequency_"+site+"_boost.csv";
	ofstream csvfile_tagfreq (filename.c_str());
	csvfile_tagfreq << "Tag; Frequency " << endl;
	for (auto it = b_tagCount[site].begin(); it != b_tagCount[site].end(); ++it)
	{
		csvfile_tagfreq << it->first <<" ; "<< it->second << endl;
	}
	csvfile_tagfreq.close();
}

void writePostScore_boost(string site)
{
	string filename="PostScore_"+site+"_boost.csv";
	ofstream csvfile_postScore (filename.c_str());
	csvfile_postScore << "Post;Score " << endl;
	for (auto it = b_postScore[site].begin(); it != b_postScore[site].end(); ++it)
	{
		csvfile_postScore << it->first <<" ; "<< it->second << endl;
	}
	csvfile_postScore.close();
}

void writePostAnswer_boost(string site)
{
	string filename="PostAnswer_"+site+"_boost.csv";
	ofstream csvfile_postAnswer (filename.c_str());
	csvfile_postAnswer << "Post;Answer " << endl;
	for (auto it = b_postAnswer[site].begin(); it != b_postAnswer[site].end(); ++it)
	{
		csvfile_postAnswer << it->first <<" ; "<< it->second << endl;
	}
	csvfile_postAnswer.close();
}

void writeUserReputation_boost(string site)
{
	string filename="UserReputation_"+site+"_boost.csv";
	ofstream csvfile_userReputation (filename.c_str());
	csvfile_userReputation << "User;Reputation " << endl;
	for (auto it = b_userReputation[site].begin(); it != b_userReputation[site].end(); ++it)
	{
		csvfile_userReputation << it->first <<" ; "<< it->second << endl;
	}
	csvfile_userReputation.close();
}

void writeUserAboutMeWords_boost(string site)
{
	string filename="UserAboutMeWords_"+site+"_boost.csv";
	ofstream csvfile_userAboutMeWords_sf (filename.c_str());
	csvfile_userAboutMeWords_sf << "User;WordCountAboutMe " << endl;
	for (auto it = b_userAboutMeWords[site].begin(); it != b_userAboutMeWords[site].end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<" ; "<< it->second << endl;
	}
	csvfile_userAboutMeWords_sf.close();
}

void writeBadges_boost(string site)
{
	string filename1="UserBadges_"+site+"_boost.csv";
	ofstream csvfile_userBadges (filename1.c_str());
	string filename2="UserBadgeCount_"+site+"_boost.csv";
	ofstream csvfile_userBadgeCount (filename2.c_str());
	csvfile_userBadges << "User;Badge " << endl;
	for (auto it = stl_userBadges[site].begin(); it != stl_userBadges[site].end(); ++it)
	{
		vector<string> badges=it->second;
		csvfile_userBadgeCount<< it->first <<" ; "<< badges.size() << endl;
		for (auto it2 = badges.begin(); it2 != badges.end(); ++it2)
		{
			csvfile_userBadges << it->first <<" ; "<< *it2 << endl;
		}

	}
	csvfile_userBadges.close();
	csvfile_userBadgeCount.close();
}

void writePostVotes_boost(string site)
{
	string filename="PostVotes_"+site+"_boost.csv";
	ofstream csvfile_userAboutMeWords_sf (filename.c_str());
	csvfile_userAboutMeWords_sf << "Post;Up;Down;Total " << endl;
	for (std::map<int , int[3]>::iterator it = stl_PostVotes[site].begin(); it != stl_PostVotes[site].end(); ++it)
	{
		csvfile_userAboutMeWords_sf << it->first <<" ; "<< it->second[0] <<" ; "<< it->second[1]<<" ; "<< it->second[2] << endl;
	}
	csvfile_userAboutMeWords_sf.close();
}

int main( int argc, char ** argv )
{
	time_t current_time;
	char* c_time_string=ctime(&current_time);
	string filename="Log"+string(c_time_string)+".log";
	ofstream log (filename.c_str());
	log<<"Virtual [kB]; RAM;CPU"<<endl;
	clock_t begin = clock();
	//pugi::xml_document doc_sf,doc_wp;
	string fromSite="serverfault";
	loadPosts(fromSite);
	log<<getVMValue()<<";"<<getRAMValue()<<";"<<getCurrentCPUValue()<<endl;
	loadPostsHistory(fromSite);
	log<<getVMValue()<<";"<<getRAMValue()<<";"<<getCurrentCPUValue()<<endl;
	loadComments(fromSite);
	log<<getVMValue()<<";"<<getRAMValue()<<";"<<getCurrentCPUValue()<<endl;
	loadUsers(fromSite);
	log<<getVMValue()<<";"<<getRAMValue()<<";"<<getCurrentCPUValue()<<endl;
	loadTags(fromSite);
	log<<getVMValue()<<";"<<getRAMValue()<<";"<<getCurrentCPUValue()<<endl;
	loadBadges(fromSite);
	log<<getVMValue()<<";"<<getRAMValue()<<";"<<getCurrentCPUValue()<<endl;
	loadVotes(fromSite);
	log<<getVMValue()<<";"<<getRAMValue()<<";"<<getCurrentCPUValue()<<endl;

	log.close();
	writeWordFrequency(fromSite);
	writeTagFrequency(fromSite);
	writePostScore(fromSite);
	writePostAnswer(fromSite);
	writeUserReputation(fromSite);
	writeUserAboutMeWords(fromSite);
	writeBadges(fromSite);
	writePostVotes(fromSite);
	/******* FIN SF ****/

	/****** WP */
	fromSite="wordpress";
	loadPosts(fromSite);
	loadPostsHistory(fromSite);
	loadComments(fromSite);
	loadUsers(fromSite);
	loadTags(fromSite);
	loadBadges(fromSite);
	loadVotes(fromSite);

	writeWordFrequency(fromSite);
	writeTagFrequency(fromSite);
	writePostScore(fromSite);
	writePostAnswer(fromSite);
	writeUserReputation(fromSite);
	writeUserAboutMeWords(fromSite);
	writeBadges(fromSite);
	writePostVotes(fromSite);

	/***** fin WP   */
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "Tiempo STL "<<elapsed_secs << endl;

	begin = clock();
	fromSite="serverfault";
	loadPosts_boost(fromSite);
	loadPostsHistory_boost(fromSite);
	loadComments_boost(fromSite);
	loadUsers_boost(fromSite);
	loadTags_boost(fromSite);
	loadBadges_boost(fromSite);
	loadVotes_boost(fromSite);

	writeWordFrequency_boost(fromSite);
	writeTagFrequency_boost(fromSite);
	writePostScore_boost(fromSite);
	writePostAnswer_boost(fromSite);
	writeUserReputation_boost(fromSite);
	writeUserAboutMeWords_boost(fromSite);
	writeBadges_boost(fromSite);
	writePostVotes_boost(fromSite);
	/******* FIN SF ****/

	/****** WP */
	fromSite="wordpress";
	loadPosts_boost(fromSite);
	loadPostsHistory_boost(fromSite);
	loadComments_boost(fromSite);
	loadUsers_boost(fromSite);
	loadTags_boost(fromSite);
	loadBadges_boost(fromSite);
	loadVotes_boost(fromSite);

	writeWordFrequency_boost(fromSite);
	writeTagFrequency_boost(fromSite);
	writePostScore_boost(fromSite);
	writePostAnswer_boost(fromSite);
	writeUserReputation_boost(fromSite);
	writeUserAboutMeWords_boost(fromSite);
	writeBadges_boost(fromSite);
	writePostVotes_boost(fromSite);
	/***** fin WP   */
	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "Tiempo Boost "<<elapsed_secs << endl;
	return EXIT_SUCCESS;
}
