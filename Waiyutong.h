#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <assert.h>
#include <regex>

#include "curl\curl.h"
#include "CJsonObject\CJsonObject.hpp"
#include "MD5.h"
#include "UrlEncode.h"

#pragma comment(lib, "libcurl.lib")

class Waiyutong
{
public:
	~Waiyutong();
	Waiyutong();
	bool Login(const char *username, const char *password);
	bool GetHomeworkByHid(const int &hid);
	bool GetLatestHomework();
	bool GetHomeworkByDate(const std::string &date);
	const std::vector< std::vector< std::string >  > GetAnswer() const;

private:
	void ParseHomeworkJson();
	const int GetLatestHomeworkHid();
	const int GetHomeworkHidByDate(std::string date, std::vector<int> *pHidVector);
	bool GetHomework(const int &hid);
	std::string Utf8ToGbk(const char *src_str);
	std::string GbkToUtf8(const char *src_str);
	bool POST(const std::string &url, const std::string &header, const std::string &postField, std::string *saveCookie, std::string *szbuffer, std::string *szheader_buffer);
	bool POST(const std::string &url, const std::string &header, const std::string &postField, std::string cookie, std::string *szbuffer, std::string *szheader_buffer);

private:
	std::vector< std::vector< std::string > > answer;
	std::string homeworkJson;
	std::string loginCookie;
	bool isLogined;
};

