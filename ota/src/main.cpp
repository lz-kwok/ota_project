#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <vector>
#include <signal.h>
#include <list>
#include <curl/curl.h>

#include "curl_base.hpp"
#include "myHttp_curl.hpp"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


using namespace std;
using namespace mycurl;
using namespace CURL_BASE;
using namespace rapidjson;

string posturl = "http://47.111.88.91:6096/iot/data/receive";


int GetMacAddress(char* mac, int len)
{
	FILE *fp = NULL;
	int ret = -1;
	char mac_t[32] = {0};
	int i,j;
	
	fp = fopen("/sys/class/net/wlan0/address","r");
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
	
	std::cout << "GetMacAddress open file failed!" << mac << std::endl;
	fclose(fp);

	return 0;
}


void *myHttp_run(void *para){
	mycurl::my_curl::curl_base curl;
	mycurl::my_curl curl_origin;
	while(1){		
			StringBuffer TEXT;
			Writer<StringBuffer> writer(TEXT);
			writer.StartObject();
			writer.Key("appkey");
			writer.String("tian jin B1 line");
			writer.Key("devicename");
			writer.String("BMS");
			writer.Key("devicekey");
			writer.Int(1);

			writer.Key("header");
			writer.StartObject();
			writer.Key("namespaceme");
			writer.String("0");
			writer.Key("version");
			writer.String("1.0");
    		writer.EndObject();

			writer.Key("payload");
			writer.StartObject();
			writer.Key("bat_volt");
			writer.Int(222);
			writer.Key("bat_soc");
			writer.Int(333);
			writer.Key("bat_min_volt");
			writer.Int(444);
			writer.Key("bcC_underV");
			writer.Double(12.34567);
    		writer.EndObject();

			writer.Key("uptime");
			writer.String("2019-06-05 17:33:38");

			writer.EndObject();
			string jsonContext = TEXT.GetString();
			std::cout << "============jsonContext============" << jsonContext << std::endl;

			string Response;
			std::list<std::string> slist{("Content-Type:application/json;charset=UTF-8")};
			CURLcode code = curl.curl_post_req(posturl,jsonContext, Response, slist, true, 10, 10);
			cout << "code:" << code << endl;
			cout << "Response:" << Response << endl;


			sleep(20);
		}

}




int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_ALL);

	pthread_t id;
    pthread_create(&id, NULL, myHttp_run, NULL);

    while (1) {
		sleep(10);
    }

    curl_global_cleanup();
    return 0;
}
