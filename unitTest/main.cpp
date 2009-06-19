#include "TestFramework.h"
#include "WebCore.h"
#include "Test_RenderSync.h"
#include "Test_RenderAsync.h"
#include "Test_EvalJavascript.h"
#include <conio.h>
#include <stdio.h>
#include <vector>

std::string getCurrentWorkingDirectory();

int main()
{
	Awesomium::WebCore* webCore = new Awesomium::WebCore(Awesomium::LOG_VERBOSE);
	webCore->setBaseDirectory(getCurrentWorkingDirectory() + "..\\media\\");

	std::vector<TestConstructor*> tests;
	tests.push_back(new Constructor<Test_RenderSync>());
	tests.push_back(new Constructor<Test_RenderAsync>());
	tests.push_back(new Constructor<Test_EvalJavascript>());

	size_t numTests = tests.size();
	size_t numPassed = 0;

	while(tests.size())
	{
		TestConstructor* testConstructor = tests.back();
		Test* test = testConstructor->Construct();
		tests.pop_back();
		delete testConstructor;

		if(test->run())
			numPassed++;

		delete test;
	}

	std::cout << (numPassed == numTests? "Testing Succeeded! " : "Testing Failed! ") << numPassed << " out of " << 
		numTests << " tests passed." << std::endl;
	std::cout << "Press any key to continue...";
	while(!kbhit()) Sleep(0);

	delete webCore;

	return 0;
}

#include <direct.h>
#include <stdlib.h>

std::string getCurrentWorkingDirectory()
{
	std::string workingDirectory = "";
	char currentPath[_MAX_PATH];
	getcwd(currentPath, _MAX_PATH);
	workingDirectory = currentPath;

	return workingDirectory + "\\";
}