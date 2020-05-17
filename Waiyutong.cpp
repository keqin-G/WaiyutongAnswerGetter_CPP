#include "Waiyutong.h"

size_t call_wirte_func(const char *ptr, size_t size, size_t nmemb, std::string *stream)
{
	size_t len = size * nmemb;
	stream->append(ptr, len);
	return len;
}

size_t header_callback(const char * ptr, size_t size, size_t nmemb, std::string * stream)
{
	size_t len = size * nmemb;
	stream->append(ptr, len);
	return len;
}

Waiyutong::~Waiyutong()
{
	curl_global_cleanup();
}

Waiyutong::Waiyutong()
{
	isLogined = false;
	curl_global_init(CURL_GLOBAL_ALL);
}

//Function: Login to http://www.waiyutong.org/User/login.html by POST
//Parameter: 'username': your Waiyutong username, 'password': your Waiyutong password
//ReturnValue: return true if succeed to login. Otherwise, return false
bool Waiyutong::Login(const char * username, const char * password)
{

	MD5 md5;
	md5.update(password);
	std::string data = "username=" + UrlEncode(GbkToUtf8(username)) + "&password=" + md5.toString();
	std::string *szbuffer = new std::string;
	std::string *szheader_buffer = new std::string;
	std::string *cookie = new std::string;
	bool ok = POST("http://www.waiyutong.org/User/login.html", "X-Requested-With: XMLHttpRequest", data, cookie, szbuffer, szheader_buffer);
	
	//get status from the returned Json, if it is '1', the login is successful
	neb::CJsonObject oJson(*szbuffer);
	if (oJson["status"].ToString() == "1" && ok)
	{
		loginCookie = *cookie;
		isLogined = true;
		return true;
	}

	isLogined = false;
	return false;
}

bool Waiyutong::GetHomeworkByHid(const int &hid)
{
	bool ok = this->GetHomework(hid);
	if (ok)
	{
		this->ParseHomeworkJson();
	}
	return ok;
}

//Function: Get homework's Json by POST to http://student.waiyutong.org/Practice/getHomeworkTests.html
//Parameter: 'hid': homework's hid.Get it from homework's Url. E.G.:https://student.waiyutong.org/Practice/homework.html?mode=free&hid=745048 the last few numbers 
//ReturnValue: return true if succeed to Get.Otherwise, return false.If not Login, return false;
//Notice: LOGIN FIRST
bool Waiyutong::GetHomework(const int & hid)
{
	if (!isLogined)
	{
		return false;
	}

	std::string *szbuffer = new std::string;
	std::string *szheader_buffer = new std::string;
	bool ok = POST("http://student.waiyutong.org/Practice/getHomeworkTests.html", "X-Requested-With: XMLHttpRequest", "hid=" + std::to_string(hid), loginCookie, szbuffer, szheader_buffer);

	neb::CJsonObject oJson(*szbuffer);
	if (oJson["status"].ToString() == "1" && ok)
	{
		//printf("%s\n", szbuffer->c_str());
		homeworkJson = *szbuffer;
		return true;
	}

	//std::cout << Utf8ToGbk(oJson["info"].ToString().c_str());

	return false;
}

bool Waiyutong::GetLatestHomework()
{
	return GetHomeworkByHid(GetLatestHomeworkHid());
}

bool Waiyutong::GetHomeworkByDate(const std::string &date)
{
	std::vector<int> v;
	int a = GetHomeworkHidByDate(date, &v);
	if (a == 0)
	{
		//std::cout << "FALSE";
		return false;
	}
	for (int i = 0; i < a; i++)
	{
		//printf("%d\n", v.at(i));

		GetHomework(v.at(i));
		this->ParseHomeworkJson();
	}
	return true;
}

//Function: Get the Latest homework's hid by POST to http://student.waiyutong.org/Homework/listDetail.html
//ReturnValue: If succeed to get, return the hid. Otherwise, return -1
const int Waiyutong::GetLatestHomeworkHid()
{
	if (!isLogined)
	{
		return false;
	}

	std::string *szbuffer = new std::string;
	std::string *szheader_buffer = new std::string;
	bool ok = POST("http://student.waiyutong.org/Homework/listDetail.html", "X-Requested-With: XMLHttpRequest", "", loginCookie, szbuffer, szheader_buffer);

	//get status from the returned Json, if it is '1', then it is successful
	neb::CJsonObject oJson(*szbuffer);
	if (oJson["status"].ToString() == "1")
	{
		std::string HID = oJson["info"]["homework"][0]["id"].ToString();
		HID = HID.substr(1, HID.size() - 2);
		//std::cout << HID;
		return std::stoi(HID);
	}
	return -1;
}
//Function: Get homework's hid from Date by POST to http://student.waiyutong.org/Homework/listDetail.html
//Parameter: 'date': the homework's assignment time.E.G. '2020-2-20'
//ReturnValue: If succeed to get, return the hid. Otherwise, return -1
const int Waiyutong::GetHomeworkHidByDate(std::string date, std::vector<int> *pHidVector)
{
	if (!isLogined)
	{
		return 0;
	}

	std::vector<int> hidVector;
	std::string *szbuffer = new std::string;
	std::string *szheader_buffer = new std::string;
	int page = 1;

	//post to get the homework's hidVector 
	if (!POST("http://student.waiyutong.org/Homework/listDetail.html", "X-Requested-With: XMLHttpRequest", "p=" + std::to_string(page), loginCookie, szbuffer, szheader_buffer))
	{
		//std::cout << "RETURN" << std::endl;
		return 0;
	}

	int pageNum = -1;
	neb::CJsonObject aJson(*szbuffer);
	if (aJson["status"].ToString() == "1")
	{
		neb::CJsonObject all = aJson["info"]["page"];
		std::string str = Utf8ToGbk(all.ToString().c_str());
		int pos = str.find("¹²");
		int ePos = str.find("Ìõ");

		std::string num = str.substr(pos + 2, ePos - pos - 1);
		pageNum = std::stoi(num);
	}
	else
	{
		return false;
	}
	//std::cout << "sz = " << *szbuffer << std::endl;
	if (pageNum % 4 == 0)
	{
		pageNum /= 4;
	}
	else
	{
		pageNum = pageNum / 4 + 1;
	}

	//std::cout << "pageNum=" << pageNum;
	while (POST("http://student.waiyutong.org/Homework/listDetail.html", "X-Requested-With: XMLHttpRequest", "p=" + std::to_string(page), loginCookie, szbuffer, szheader_buffer) && page < pageNum)
	{
		std::cout << "page=" << page<<std::endl;
		//get status from the returned Json, if it is '1', then it is successful
		neb::CJsonObject oJson(*szbuffer);
		neb::CJsonObject all = oJson["info"]["homework"];
		
		if (!(oJson["status"].ToString() == "1"))
		{
			continue;
		}
		for (int i = 0; i < all.GetArraySize(); i++)
		{
			//std::cout << all[i].ToString() << std::endl;
			if (all[i]["start_time"].ToString().find(date) != -1)
			{
				std::string HID = all[i]["id"].ToString();
				HID = HID.substr(1, HID.size() - 2);
				//std::cout << "HID=" << HID << std::endl;
				hidVector.push_back(std::stoi(HID));
			}
		}
		
		szbuffer = new std::string;
		szheader_buffer = new std::string;
		page++;
	}

	*pHidVector = hidVector;
	return hidVector.size();
}

const std::vector< std::vector< std::string>  > Waiyutong::GetAnswer() const
{
	return this->answer;
}

//This is a private function
//Function: Parse the answer from homeworkJson.And save them in the vector 'Answer'
void Waiyutong::ParseHomeworkJson()
{
	neb::CJsonObject oJson(homeworkJson);
	std::vector<std::string> answer0;
	std::vector<std::string> answer1;
	std::vector<std::string> answer2;
	std::vector<std::string> answer3;
	std::vector<std::string> answer4;

	int i = 0;
	while (i < 10)
	{
		std::string all = Utf8ToGbk(oJson["info"]["parseTests"][i]["parse_test"].ToString().c_str());

		if (i >= 0 && i < 5)
		{
			std::regex pattern("<p class=\\\\\\\"right_answer_class\\\\\\\" data-right-answer=\\\\\\\"(.)\\\\\\\">");
			std::smatch result;
			bool ok = std::regex_search(all, result, pattern);
			answer0.push_back(result.str(1));
			if (i == 4)
			{
				answer0.push_back("\r\n");
			}
			//printf("%s\n", result.str(1));
		}
		else if (i == 5)
		{
			std::smatch result;
			std::regex pattern("<p class=\\\\\\\"right_answer_class\\\\\\\" data-right-answer=\\\\\\\"(.)\\\\\\\">");

			std::string::const_iterator itBegin = all.begin();
			std::string::const_iterator itEnd = all.end();
			while (regex_search(itBegin, itEnd, result, pattern))
			{
				answer1.push_back(result.str(1));
				itBegin = result[0].second;
			}
			answer1.push_back("\r\n");
		}
		else if (i == 6)
		{
			std::smatch result;
			std::regex pattern("<p class=\\\\\\\"right_answer_class\\\\\\\" data-right-answer=\\\\\\\"(.)\\\\\\\">");

			std::string::const_iterator itBegin = all.begin();
			std::string::const_iterator itEnd = all.end();
			while (regex_search(itBegin, itEnd, result, pattern))
			{
				answer2.push_back(result.str(1));
				itBegin = result[0].second;
			}
			answer2.push_back("\r\n\r\n");
		}
		else if (i == 7)
		{
			i++;
			continue;
		}
		else if (i == 8)
		{
			std::smatch result;
			std::regex pattern("<div class=\\\\\"speak_sentence no_audio answer enable\\\\\" data-mp3=\\\\\".{0,20}.mp3\\\\\" data-starttime=[0-9] data-endtime=[0-9] data-text=\\\\\"(.*?)\\\\\"> answer:&nbsp;&nbsp;(.*?)</div>");

			std::string::const_iterator itBegin = all.begin();
			std::string::const_iterator itEnd = all.end();

			while (regex_search(itBegin, itEnd, result, pattern))
			{
				answer3.push_back(result.str(1) + "\r\n");
				itBegin = result[0].second;
			}
			answer3.push_back("\r\n");
		}
		else if (i == 9)
		{
			std::smatch result;
			std::regex pattern("<span class=\\\\\"speak_sentence enable\\\\\" data-mp3=\\\\\".[0-9]*.mp3\\\\\" data-starttime=.[0-9]* data-endtime=.[0-9]*>(.*?)</span>");

			std::string::const_iterator itBegin = all.begin();
			std::string::const_iterator itEnd = all.end();

			while (regex_search(itBegin, itEnd, result, pattern))
			{
				answer4.push_back(result.str(1) + "\r\n");
				itBegin = result[0].second;
			}
			answer4.push_back("\r\n");
		}
		i++;
	}
	answer.push_back(answer0);
	answer.push_back(answer1);
	answer.push_back(answer2);
	answer.push_back(answer3);
	answer.push_back(answer4);

}

//This is a private function
//Function: convert utf8 to gbk
std::string Waiyutong::Utf8ToGbk(const char *src_str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}

//This is a private function
//Function: convert gbk to utf8
std::string Waiyutong::GbkToUtf8(const char *src_str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	std::string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

bool Waiyutong::POST(const std::string &url, const std::string &header, const std::string &postField, std::string cookie, std::string *szbuffer, std::string *szheader_buffer)
{
	CURL *curl_handle = NULL;
	CURLcode res = CURLE_FAILED_INIT;
	struct curl_slist *chunk = NULL;
	//set headers context
	chunk = curl_slist_append(chunk, header.c_str());
	curl_handle = curl_easy_init();

	//set headers
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, chunk);
	//set post url
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	//set login cookie
	curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST, cookie.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, FALSE);
	//Get web content, call back function  
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, call_wirte_func);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, szbuffer);
	//Get web headers, call back function 
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, szheader_buffer);
	//set from-data
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postField.c_str());
	res = curl_easy_perform(curl_handle);
	curl_easy_cleanup(curl_handle);

	if (res == CURLE_OK)
	{
		return true;
	}
	return false;
}

bool Waiyutong::POST(const std::string &url, const std::string &header, const std::string &postField, std::string *saveCookie, std::string *szbuffer, std::string *szheader_buffer)
{
	CURL *curl_handle = NULL;
	CURLcode res = CURLE_FAILED_INIT;

	struct curl_slist *chunk = NULL;
	//set headers context
	chunk = curl_slist_append(chunk, "X-Requested-With: XMLHttpRequest");
	curl_handle = curl_easy_init();

	//set headers
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, chunk);
	//set POST url
	curl_easy_setopt(curl_handle, CURLOPT_URL, "http://www.waiyutong.org/User/login.html");
	//save Cookie
	curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR, saveCookie);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, FALSE);
	//Get web content, call back function 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, call_wirte_func);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, szbuffer);
	//Get web header, call back function   
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, szheader_buffer);
	//set from-data
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postField.c_str());
	res = curl_easy_perform(curl_handle);

	//parse and save cookie
	res = curl_easy_getinfo(curl_handle, CURLINFO_COOKIELIST, &chunk);
	while (chunk)
	{
		if (saveCookie->length() > 0)
			*saveCookie += "\r\n";
		*saveCookie += chunk->data;
		chunk = chunk->next;
	}

	curl_easy_cleanup(curl_handle);

	if (res == CURLE_OK)
	{
		return true;
	}
	return false;
}

