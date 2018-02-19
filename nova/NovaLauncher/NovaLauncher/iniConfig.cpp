#include "iniConfig.h"

std::string intToString(const int& number)
{
	std::ostringstream oss;
	oss << number;
	return oss.str();
}
std::string LoadConfigString(const char *pszKey, const char *pszItem, const char *pszDefault)
{
	char buffer[MAX_PATH];
	GetPrivateProfileStringA(pszKey, pszItem, pszDefault ? pszDefault : "", buffer, MAX_PATH, configPath.c_str());
	return std::string(buffer);
}
int LoadConfigInt(const char *pszKey, const char *pszItem, const int iDefault)
{
	return GetPrivateProfileIntA(pszKey, pszItem, iDefault, configPath.c_str());
}
int WriteConfigString(const char *pszKey, const char *pszItem, const char *pszValue)
{
	return WritePrivateProfileStringA(pszKey, pszItem, pszValue, configPath.c_str());
}