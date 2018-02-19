#pragma once

#include "Search/MCTSCD.h"

class HTMLoutput
{
public:
	HTMLoutput(MCTSCD::gameNode_t* node, EvaluationFunction* ef);
	
private:
	std::ofstream htmlFile;
	EvaluationFunction* _ef;
	std::stringstream rootName;

	void createFile(std::string fileName, std::string rootPath);
	void printMCTSCDnode(MCTSCD::gameNode_t* node);
	void closeFile();
};
