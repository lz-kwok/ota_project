#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <vector>
#include <signal.h>
#include <list>
// #include <curl/curl.h>
#include <curl64/curl.h>

#include "curl_base.hpp"
#include "myHttp_curl.hpp"

#include "md5.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


using namespace std;
using namespace mycurl;
using namespace CURL_BASE;
using namespace rapidjson;

struct hal_timeval{
  long    tv_sec;         /* seconds */
  long    tv_usec;        /* and microseconds */
};

typedef struct {
	char mac[64];	
	char current_ver[32];
}OTAManager;
static OTAManager mOtamanager;

#define downLoad_url 	"http://47.111.88.91:6096/downloadExample/update.tar.gz"
#define downLoad_file 	"/home/myDevelop/ota_project/ota/bin/update.tar.gz"
#define ota_conf_path   "/home/myDevelop/ota_project/ota/bin/ota.conf"
#define SALT_KEY 		"f3c05205bb284a8b464c662b08f5d864"
#define URL_POST		"https:/"

string posturl = "http://47.111.88.91:6096/iot/data/receive";

static uint32_t mLastTimems = 0;
static struct hal_timeval mTimeVal;

int GetMacAddress(char* mac, int len)
{
	FILE *fp = NULL;
	int ret = -1;
	char mac_t[32] = {0};
	int i,j;
	
	fp = fopen("/sys/class/net/wlp5s0/address","r");
	if(fp==NULL){
		std::cout << "GetMacAddress open file failed!" << std::endl;
		return -1;
	}
	
	ret = fread(mac_t,1,17,fp);
	if(ret < 0){
		std::cout << "GetMacAddress open file failed!" << std::endl;
		fclose(fp);
		return -1;
	}
	
	for(i = 0, j = 0; mac_t[i] != 0; i++){
		if (mac_t[i] != ':') {
			if (mac_t[i] >= 'A' && mac_t[i] <= 'Z'){
				mac_t[j++] = mac_t[i]-'A' + 'a';				
			}else {
				mac_t[j++] = mac_t[i];
			}		
		}
	}	
	mac_t[j] = '\0';
	strncpy(mac, mac_t, len);
	
	std::cout << "GetMacAddress  " << mac << std::endl;
	fclose(fp);

	return 0;
}

int GetCurrenVersion(char* ver, int len)
{
	int ret = -1;
	char ver_t[32] = {0};
	memset(ver_t,0x0,32);
	
	int fd = open(ota_conf_path, O_RDONLY);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}
	ret = read(fd, ver_t, 16);
	if (-1 == ret)
	{
		perror("read");
		close(fd);
		return -1;
	}
	
	ver_t[ret] = '\0';
	strncpy(ver, ver_t, ret-1);
	printf("%s %s\r\n",__func__,ver);

	close(fd);
	return 0;
}

void GetTimeOfDay(struct hal_timeval* tv)
{
    uint32_t timems = 0; //it will roll over every 49 days, 17 hours.
    uint32_t timediff = 0;

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC,&ts);
	timems = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    if (timems < mLastTimems) {
        uint32_t maxTime = -1;

        timediff = maxTime - mLastTimems;
        timediff += timems;
    } else {
        timediff = timems - mLastTimems;
    }

    mLastTimems = timems;
	if (mTimeVal.tv_usec == 0 && mTimeVal.tv_sec == 0) {
		mTimeVal.tv_sec = timediff / 1000;
		mTimeVal.tv_usec = (timediff % 1000) * 1000;
	}else {
		mTimeVal.tv_usec += timediff * 1000;//ms = us * 1000
		if (mTimeVal.tv_usec >= 1000000) { // 1 second
			mTimeVal.tv_sec += mTimeVal.tv_usec / 1000000;
			mTimeVal.tv_usec = mTimeVal.tv_usec % 1000000;
		}
	}

    tv->tv_sec = mTimeVal.tv_sec;
    tv->tv_usec = mTimeVal.tv_usec;
}


void *myOTA_run(void *para){
	mycurl::my_curl::curl_base curl;
	// mycurl::my_curl curl_origin;
	
	memset(&mOtamanager, 0x0, sizeof(mOtamanager));
	GetMacAddress(mOtamanager.mac, 64);
	GetCurrenVersion(mOtamanager.current_ver,32);

	// curl.Download(downLoad_url,downLoad_file);
	// curl.DownloadFinish();
	while(1){		
		StringBuffer VER_STR;
		Writer<StringBuffer> writer(VER_STR);
		writer.StartObject();
		writer.Key("version");
		writer.String(mOtamanager.current_ver);
		writer.EndObject();
		string jsonContext = VER_STR.GetString();
		printf("%s\r\n",jsonContext.c_str());

		char request_url[256];
		unsigned char sign[128];
		char md5_str[MD5_STR_LEN + 1];
		memset(sign,0x0,128);
		memset(md5_str,0x0,MD5_STR_LEN + 1);
		struct hal_timeval now; 
		GetTimeOfDay(&now);
		sprintf((char *)sign,"mac%stime%u%s", mOtamanager.mac,(uint32_t)now.tv_sec, SALT_KEY);
		Compute_string_md5(sign, strlen((const char*)sign), md5_str);
		sprintf(request_url,"%s?mac=%s&time=%u&sign=%s",URL_POST,mOtamanager.mac,(uint32_t)now.tv_sec,md5_str);
		printf("request_url:[%s]\r\n",request_url);
		//(request_url, 256, "%s?appkey=%s&device_id=%s&time=%u&sign=%s", url, APP_KEY,  mManager.device_id, (ev_uint32_t)now.tv_sec, sign);
		// writer.Key("devicename");
		// writer.String("BMS");
		// writer.Key("devicekey");
		// writer.Int(1);

		// writer.Key("header");
		// writer.StartObject();
		// writer.Key("namespaceme");
		// writer.String("0");
		// writer.Key("version");
		// writer.String("1.0");
		// writer.EndObject();

		// writer.Key("payload");
		// writer.StartObject();
		// writer.Key("bat_volt");
		// writer.Int(222);
		// writer.Key("bat_soc");
		// writer.Int(333);
		// writer.Key("bat_min_volt");
		// writer.Int(444);
		// writer.Key("bcC_underV");
		// writer.Double(12.34567);
		// writer.EndObject();

		// writer.Key("uptime");
		// writer.String("2019-06-05 17:33:38");

		// writer.EndObject();
		// string jsonContext = TEXT.GetString();
		// std::cout << "============jsonContext============" << jsonContext << std::endl;

		// string Response;
		// std::list<std::string> slist{("Content-Type:application/json;charset=UTF-8")};
		// CURLcode code = curl.curl_post_req(posturl,jsonContext, Response, slist, true, 10, 10);
		// cout << "code:" << code << endl;
		// cout << "Response:" << Response << endl;


		sleep(2);
		printf("%s\r\n",__func__);
	}
}




int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_ALL);

	pthread_t id;
    pthread_create(&id, NULL, myOTA_run, NULL);

    while (1) {
		sleep(10);
    }

    curl_global_cleanup();
    return 0;
}
