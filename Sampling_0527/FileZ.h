#ifndef _FILE_H
#define _FILE_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "io.h"
#include "direct.h"

using namespace std;

class FileZ
{
public:
	FileZ(string name);
	bool isExist();
	void getFiles();
	void getFiles(string name);
	void getFiles(string name, string type);
	void copyFile(string target);
	void copyFile(string source, string target);
	void writeResult(string path,string flag);
public:
	string name;
	vector<string> subfile;
	vector<string> subpath;
};

#endif // !_FILE_H