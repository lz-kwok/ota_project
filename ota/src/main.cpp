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
