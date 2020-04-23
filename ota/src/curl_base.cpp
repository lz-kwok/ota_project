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
#include "curl_base.hpp"

using namespace std;



namespace CURL_BASE{

curl_base::curl_base(void)
{
    cout << "curl_base Object is being created" << endl;
}


size_t req_reply(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (stream == NULL || ptr == NULL || size == 0)
        return 0;
 
    size_t realsize = size * nmemb;
    std::string *buffer = (std::string*)stream;
    if (buffer != NULL)
    {
        buffer->append((const char *)ptr, realsize);
    }

    // size_t nWrite = fwrite(ptr, 1, nmemb, stream);  
    return realsize;
    /*
    std::string *str = (std::string*)stream;
    (*str).append((char*)ptr, size*nmemb);
    return size * nmemb;
    */
}


static size_t OnDownLoadFile(void* buffer,size_t size, size_t nmemb,void* fp)
{
   fwrite(buffer, size, nmemb, (FILE*)fp);
   fflush((FILE*)fp);

   return (size*nmemb);
}

static int my_progress_func(char *progress_data,
                     double t, /* dltotal */
                     double d, /* dlnow */
                     double ultotal,
                     double ulnow)
{
  printf("%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);
  return 0;
}

CURLcode curl_base::curl_get_req(const std::string &url, std::string &response,std::list<std::string> listRequestHeader,
						bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout)
{
    // init curl  
    CURL *curl = curl_easy_init();
    // res code  
    CURLcode res;
    if (curl)
    {
        // set params  
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // url  
        //curl_easy_setopt(m_curl, CURLOPT_PORT, 8089);    //port
        curl_easy_setopt(curl, CURLOPT_POST, 0); // get reqest 
        //构建HTTP报文头
        struct curl_slist* headers = NULL;
        if (listRequestHeader.size() > 0)
        {
            std::list<std::string>::iterator iter, iterEnd;
            iter = listRequestHeader.begin();
            iterEnd = listRequestHeader.end();
            for (iter; iter != iterEnd; iter++)
            {
                headers = curl_slist_append(headers, iter->c_str());
            }
            //headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
            //headers = curl_slist_append(headers, "Content-Type:application/x-www-form-urlencoded");
            if (headers != NULL)
            {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);		//设置http请求头信息
            }
        }
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); 				// if want to use https  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); 				// set peer and host verify false  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        if (bResponseIsWithHeaderData)
        {
            curl_easy_setopt(curl, CURLOPT_HEADER, 1);						//响应体中是否包含了头信息，比如Content-Type:application/json;charset=UTF-8
        }
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, nConnectTimeout); 	// set transport and time out time  
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, nTimeout);
        // start request  
        res = curl_easy_perform(curl);
        if (headers != NULL)
        {
            curl_slist_free_all(headers); //free the list again
        }
    }
    // release curl  
    curl_easy_cleanup(curl);
    return res;
}



CURLcode curl_base::curl_post_req(const std::string &url, const std::string &postParams, std::string &response, std::list<std::string> listRequestHeader, 
							bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout)
{
    // init curl  
    CURL *curl = curl_easy_init();
    // res code  
    CURLcode res;
    if (curl)
    {
        // set params  
        curl_easy_setopt(curl, CURLOPT_POST, 1); 			// post req  
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); 	// url  
        //curl_easy_setopt(m_curl, CURLOPT_PORT, 8089);    	//port
        //构建HTTP报文头
        struct curl_slist* headers = NULL;
        if (listRequestHeader.size() > 0)
        {
            std::list<std::string>::iterator iter, iterEnd;
            iter = listRequestHeader.begin();
            iterEnd = listRequestHeader.end();
            for (iter; iter != iterEnd; iter++)
            {
                headers = curl_slist_append(headers, iter->c_str());
            }
            if (headers != NULL)
            {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);//设置http请求头信息
            }
        }
        else
        {
            //Content-Type:application/x-www-form-urlencoded
            headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
            if (headers != NULL)
            {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);	//设置http请求头信息
            }
        }
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str()); // params  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); 			// if want to use https  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); 			// set peer and host verify false  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 				//返回的头部中有Location(一般直接请求的url没找到)，则继续请求Location对应的数据 
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        if (bResponseIsWithHeaderData)
        {
            curl_easy_setopt(curl, CURLOPT_HEADER, 1);					//响应体中是否包含了头信息，比如Content-Type:application/json;charset=UTF-8
        }
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, nConnectTimeout);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, nTimeout);
        // start request  
        res = curl_easy_perform(curl);
        if (headers != NULL)
        {
            curl_slist_free_all(headers); 								//free the list again
        }
    }
    // release curl  
    curl_easy_cleanup(curl);
	
    return res;
}

void curl_base::private_post_print(void){
    cout << "private_post_print:this is a post test" << endl;
}



CURLcode curl_base::Download(std::string strUrl,std::string filepath)
{
    CURLcode res;
    char *progress_data = "* ";  
    FILE* fp = fopen(filepath.c_str(),"wb+");

    curl_handle = curl_easy_init();

    /* send all data to this function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, OnDownLoadFile);
    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)fp);

    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);  
    curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, my_progress_func);  
    curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, progress_data);

    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl_handle,CURLOPT_RESUME_FROM,0);  //从0字节开始下载
    /* 设置连接超时,单位:毫秒 */
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, 10000L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 10000L);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 3);
    curl_easy_setopt(curl_handle, CURLOPT_URL, const_cast<char*>(strUrl.c_str()));


    /* get it! */
    res = curl_easy_perform(curl_handle);
    /* check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    fclose(fp);

    return res;
}

void curl_base::DownloadFinish()
{
    curl_easy_cleanup(curl_handle);
}

}