#pragma once

#include <string>
#include <windows.h>
#include <iostream>
#include <sstream>

extern std::string configPath;

std::string intToString(const int& number);
std::string LoadConfigString(const char *pszKey, const char *pszItem, const char *pszDefault = NULL);
int LoadConfigInt(const char *pszKey, const char *pszItem, const int iDefault = 0);
int WriteConfigString(const char *pszKey, const char *pszItem, const char *pszValue);
