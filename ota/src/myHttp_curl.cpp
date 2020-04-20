#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <signal.h>
// #include <curl/curl.h>
#include <curl64/curl.h>
#include <list>
#include "myHttp_curl.hpp"

using namespace std;
//using namespace mycurl;

namespace mycurl{

my_curl::my_curl(void)
{
    cout << "my_curl Object is being created" << endl;
}

size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (stream == NULL || ptr == NULL || size == 0)
        return 0;
 
    size_t realsize = size * nmemb;
    std::string *buffer = (std::string*)stream;
    if (buffer != NULL)
    {
        buffer->append((const char *)ptr, realsize);
    }
    return realsize;
    /*
    std::string *str = (std::string*)stream;
    (*str).append((char*)ptr, size*nmemb);
    return size * nmemb;
    */
}


CURLcode my_curl::curl_get_req(const std::string &url, std::string &response,std::list<std::string> listRequestHeader,
						bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout)
{
    cout << "hello world" << endl;    
}



CURLcode my_curl::curl_post_req(const std::string &url, const std::string &postParams, std::string &response, std::list<std::string> listRequestHeader, 
							bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout)
{
    cout << "hello world" << endl; 
}

CURLcode my_curl::curl_download_init()
{
    cout << "hello world" << endl; 

}

void my_curl::SetDwonloadCallback(Callback cb)
{
    cout << "hello world" << endl; 

}

CURLcode my_curl::Download(std::string strUrl)
{
    cout << "hello world" << endl; 

}

}