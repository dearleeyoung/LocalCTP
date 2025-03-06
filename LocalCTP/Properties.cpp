#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include "Properties.h"

// Properties.cpp is from quantcast/qfs(https://github.com/quantcast/qfs), and add some comments and other changes

Properties::Properties()
: propmap()
{
}

Properties::Properties(const Properties &p)
: propmap(p.propmap)
{
}

Properties::~Properties()
{
}

int Properties::loadProperties(const char* fileName, char delimiter, bool verbose, bool multiline /*=false*/)
{
    setlocale(LC_ALL,"Chinese-simplified");//设置中文环境
	std::ifstream input(fileName);
    setlocale(LC_ALL,"C");//还原
	std::string line;

	if(!input.is_open()) 
	{
		std::cerr <<  "Properties::loadProperties() Could not open the file:" << fileName << std::endl;
		return(-1);
	}
	loadProperties(input, delimiter, verbose, multiline);
	input.close();
	return 0;
}

inline void removeLTSpaces(std::string& str){

	char const* const delims = " \t\r\n";

	// trim leading whitespace
	std::string::size_type const notwhite = str.find_first_not_of(delims);
	str.erase(0,notwhite);
	if (notwhite != std::string::npos) { // Do not assume that string::npos+1 == 0
		// trim trailing whitespace
		str.erase(str.find_last_not_of(delims)+1);
	}
}

int Properties::loadProperties(std::istream &ist, char delimiter, bool verbose,bool multiline /*=false*/)
{
	std::string line = "";
	while(ist)
	{
		line = "";
		getline(ist, line);                         //read one line at a time
		if  (line.find('#') == 0)
			continue;                               //ignore comments
		std::string::size_type pos =
			line.find(delimiter);                   //find the delimiter

		if( pos == line.npos )
			continue;                               //ignore if no delimiter is found
		std::string key = line.substr(0,pos);       // get the key
		removeLTSpaces(key);
		std::string value = line.substr(pos+1);     //get the value
		removeLTSpaces(value);

		if (multiline)
			propmap[key] += value;                  // allow properties to be spread across multiple lines
		else
			propmap[key] = value;

		//if( verbose)
		//	std::cout << "Loading key " << key  << " with value " << propmap[key] << std::endl;
	}
	return 0;
}


void Properties::setValue(const std::string key, const std::string value) {
	propmap[key] = value;
	return;
}

std::string Properties::getValue(std::string key, std::string def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : i->second);
}
const char* Properties::getValue(std::string key, const char* def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : i->second.c_str());
}

int Properties::getValue(std::string key, int def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : atoi(i->second.c_str()));
}

unsigned int Properties::getValue(std::string key, unsigned int def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : atoi(i->second.c_str()));
}

long Properties::getValue(std::string key, long def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : atol(i->second.c_str()));
}

unsigned long Properties::getValue(std::string key, unsigned long def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : atol(i->second.c_str()));
}

long long Properties::getValue(std::string key, long long def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : atol(i->second.c_str()));
}

unsigned long long Properties::getValue(std::string key, unsigned long long def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : atol(i->second.c_str()));
}

double Properties::getValue(std::string key, double def) const
{
	PropMap::const_iterator const i = propmap.find(key);
	return (i == propmap.end() ? def : atof(i->second.c_str()));
}

void Properties::getList(std::string &outBuf, std::string linePrefix, std::string lineSuffix) const
{
	PropMap::const_iterator iter;
	for (iter = propmap.begin(); iter != propmap.end(); iter++) {
		if (iter->first.size() > 0) {
			outBuf += linePrefix;
			outBuf += iter->first;
			outBuf += '=';
			outBuf += iter->second;
			outBuf += lineSuffix;
		}
	}
	return;
}

void Properties::copyWithPrefix(std::string prefix, Properties& props) const
{
	PropMap::const_iterator iter;
	for (iter = propmap.begin(); iter != propmap.end(); iter++) {
		const std::string& key = iter->first;
		if (key.compare(0, prefix.length(), prefix) == 0) {
			props.propmap[key] = iter->second;
		}
	}
}