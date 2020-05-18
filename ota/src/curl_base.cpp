#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
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

    return realsize;
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
    static double download_d = 0.0;
    if(download_d != d){
        download_d = d;
        printf("%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);
    }
    return 0;
}

static size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream) {
       int r;
       long len = 0;
 
        r = sscanf((const char*)ptr, "Content-Length: %ld\n", &len);
        if (r) /* Microsoft: we don't read the specs */
            *((long *) stream) = len;
 
       return size * nmemb;
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
    curl_off_t local_file_len = -1 ;
    long filesize =0 ;
    
    CURLcode r = CURLE_GOT_NOTHING;
    int c;
    struct stat file_info;
    int use_resume = 0;
    char *progress_data = "* ";  

    if(stat(filepath.c_str(), &file_info) == 0) 
    {
        local_file_len =  file_info.st_size;
        use_resume  = 1;
    }


    FILE* fp = fopen(filepath.c_str(),"ab+");

    curl_handle = curl_easy_init();

    /* send all data to this function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, OnDownLoadFile);
    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)fp);

    //设置http 头部处理函数
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &filesize);
    //设置文件续传的位置给libcurl
    curl_easy_setopt(curl_handle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);

    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);  
    curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, my_progress_func);  
    curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, progress_data);

    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl_handle,CURLOPT_RESUME_FROM,0L);  //从0字节开始下载
    /* 设置连接超时,单位:毫秒 */
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, 100000L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 100000L);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 3);
    curl_easy_setopt(curl_handle, CURLOPT_URL, const_cast<char*>(strUrl.c_str()));

    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

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


CURLcode curl_base::Upload(std::string strUrl,std::string filepath)
{
    CURL* curl = NULL;
    CURLcode res;
    string response;
    long httpcode;
 
    CURLM *multi_handle;  
    int still_running;  
    
    struct curl_httppost *formpost=NULL;  
    struct curl_httppost *lastptr=NULL;  
    struct curl_slist *headerlist=NULL;  
    static const char buf[] = "Expect:";  
    
    /* Fill in the file upload field. This makes libcurl load data from 
        the given file name when curl_easy_perform() is called. */  
    curl_formadd(&formpost,  
                &lastptr,  
                CURLFORM_COPYNAME, "uploadLog",  
                CURLFORM_FILE, const_cast<char*>(filepath.c_str()),  
                CURLFORM_END);  
    
    /* Fill in the filename field */  
    curl_formadd(&formpost,  
                &lastptr,  
                CURLFORM_COPYNAME, "filename",  
                CURLFORM_COPYCONTENTS, "ota.conf",  
                CURLFORM_END);  
    
    curl_formadd(&formpost,  
                &lastptr,  
                CURLFORM_COPYNAME, "submit",  
                CURLFORM_COPYCONTENTS, "send",  
                CURLFORM_END);  
    
    curl = curl_easy_init();  
    multi_handle = curl_multi_init();  
    
    /* initalize custom header list (stating that Expect: 100-continue is not 
        wanted */  
    headerlist = curl_slist_append(headerlist, buf);  
    if(curl && multi_handle) {  
    
        /* what URL that receives this POST */  
        curl_easy_setopt(curl, CURLOPT_URL, const_cast<char*>(strUrl.c_str()));  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);  
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);  

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
        curl_easy_setopt(curl, CURLOPT_HEADER, 1);
        
        curl_multi_add_handle(multi_handle, curl);  
        
        curl_multi_perform(multi_handle, &still_running);  
        
        do {  
            struct timeval timeout;  
            int rc; /* select() return code */  
        
            fd_set fdread;  
            fd_set fdwrite;  
            fd_set fdexcep;  
            int maxfd = -1;  
        
            long curl_timeo = -1;  
        
            FD_ZERO(&fdread);  
            FD_ZERO(&fdwrite);  
            FD_ZERO(&fdexcep);  
        
            /* set a suitable timeout to play around with */  
            timeout.tv_sec = 1;  
            timeout.tv_usec = 0;  
        
            curl_multi_timeout(multi_handle, &curl_timeo);  
            if(curl_timeo >= 0) {  
            timeout.tv_sec = curl_timeo / 1000;  
            if(timeout.tv_sec > 1)  
                timeout.tv_sec = 1;  
            else  
                timeout.tv_usec = (curl_timeo % 1000) * 1000;  
            }  
        
            /* get file descriptors from the transfers */  
            curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);  
        
            /* In a real-world program you OF COURSE check the return code of the 
                function calls.  On success, the value of maxfd is guaranteed to be 
                greater or equal than -1.  We call select(maxfd + 1, ...), specially in 
                case of (maxfd == -1), we call select(0, ...), which is basically equal 
                to sleep. */  
        
            rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);  
        
            switch(rc) {  
                case -1:  
                    /* select error */  
                break;  
                case 0:  
                default:  
                    /* timeout or readable/writable sockets */  
                    printf("upload!\n");  
                    curl_multi_perform(multi_handle, &still_running);  
                    printf("running: %d!\n", still_running);  
                break;  
            }  
        } while(still_running);  

        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode);
        printf("code [%d] \r\nreponse header \r\n[\r\n\r\n%s]\r\n",httpcode,response.c_str());
        
        curl_multi_cleanup(multi_handle);  
        
        /* always cleanup */  
        curl_easy_cleanup(curl);  
        
        /* then cleanup the formpost chain */  
        curl_formfree(formpost);  
        
        /* free slist */  
        curl_slist_free_all (headerlist);  
    }  
    
    return res;
}

}