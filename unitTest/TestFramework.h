#ifndef __TestFramework_H__
#define __TestFramework_H__

#include <string>
#include <fstream>
#include <iostream>
#include "Timer.h"

#define LOG(X) std::cout << X << std::endl;

inline void logTestValue(const std::string& testName, double value)
{
	std::string path = "testing\\TESTDATA_" + testName + ".js";

	std::ifstream inputfile(path.c_str());

	bool shouldAppend = false;
	if(inputfile.is_open() && inputfile.good())
		shouldAppend = true;

	inputfile.close();

	std::fstream file;

	if(shouldAppend)
		file.open(path.c_str());
	else
		file.open(path.c_str(), std::fstream::out|std::fstream::trunc);

	if(shouldAppend)
	{
		char c;
		while(true)
		{
			file.get(c);
			
			if(!file.good())
			{
				LOG("Could not log value for test '" + testName +
					"', reached end of file while trying to append value.");
				break;
			}
			else if(c == ';')
			{
				file.seekp((int)file.tellg() - 3);
				file << "[" << 1000*time(NULL) << ", " << value << "]," << std::endl;
				file << "];";
				break;
			}
		}
	}
	else
	{
		file << "var " + testName + " = [" << std::endl;
		file << "[" << 1000*time(NULL) << ", " << value << "]," << std::endl;
		file << "];";
	}

	file.close();
}

class Test
{
	std::string testName;
public:
	Test(const std::string& name) : testName(name)
	{
		log("Created");
	}

	~Test()
	{
		log("Destroyed");	
	}

	virtual bool run() = 0;

	inline void log(const std::string& str)
	{
		std::cout << "Test[" << testName << "] : " << str << std::endl;
	}
};

struct TestConstructor
{
	virtual Test* Construct() = 0;
};

template<class TestType>
struct Constructor : TestConstructor
{
	Test* Construct()
	{
		return new TestType();
	}
};

#endif