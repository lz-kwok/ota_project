#include <iostream>
#include <fstream>
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
#include "kc_ferrero.hpp"

#include "md5.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


using namespace std;
using namespace kc_ferrero;
using namespace rapidjson;


kc_ferrero::kc_ferrero::curl_base curl;
kc_ferrero::kc_ferrero my_ferrero;

typedef enum {
	Type_STRING = 0,
	Type_INT,
	Type_BOOL,
	Type_ARRAY
}Json_Type;


enum DEVICE_FERRERO_STATE{
	DEVICE_LOGIN_PLATFORM,		
	DEVICE_REQUIRE_ACCESS_RIGHT,
	DEVICE_PROCESS_READY,
	DEVICE_OTA_PROCESS
};

struct hal_timeval{
  long    tv_sec;         /* seconds */
  long    tv_usec;        /* and microseconds */
};



typedef struct {
	uint8_t processStatus;
	string token;
	string username;
	string password;
	int verCode;
	string verString;
	string otaurl;
	string md5val;
	string todayDate;
	int upLoadIndex[3];
	string timestamp;
	struct hal_timeval upLoadTime;
	string pl_name[3];
}mManager;
static mManager _mManager;



#define downLoad_url 	"http://47.111.88.91:6096/downloadExample/update.tar.gz"
#define downLoad_file 	"/home/myDevelop/ota_project/ota/bin/update.tar.gz"
#define ota_conf_path   "/home/myDevelop/ota_project/ota/bin/ota.conf"
#define SALT_KEY 		"f3c05205bb284a8b464c662b08f5d864"
#define URL_POST		"https:/"

#define CONFIG_PATH     		"/home/myDevelop/ota_project/ota/bin/config.json"
#define CONFIG_PATH_BACKUP     	"/home/myDevelop/ota_project/ota/bin/config_backup.json"
#define NFS_PATH				"/home/leon/Downloads/develop/split/"


// string posturl = "http://47.111.88.91:6096/iot/data/receive";
string posturl = "30000iot.cn:9001/api/Upload/data/";
string uploadurl = "https://test-dttqa.ferrero.com.cn/gateway/api/saas-master-data/alert_log/uploadLogFile";
string getlogintoken = "https://test-dttqa.ferrero.com.cn/gateway/api/saas-core-tenant/authentication/device/login";
string getaccessright = "https://test-dttqa.ferrero.com.cn/gateway/api/saas-core-tenant/authentication/device/user";
string getupdate = "https://test-dttqa.ferrero.com.cn/gateway/api/saas-master-data/version_manager/version/CONNEXT_MAGIC_BOX?";


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

void GetTimeOf_Day(struct hal_timeval* tv)
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

std::string ReadConfigfile(const char *config_path)
{
	std::ifstream tmp(config_path);
  	std::string str((std::istreambuf_iterator<char>(tmp)),
                  std::istreambuf_iterator<char>());

	rapidjson::Document document;
  	document.Parse(str.c_str());
	
	if (document.HasParseError()) {		//判断配置文件json格式是否被破坏
		return "";
	}

	if(document.HasMember("nfs_server")){
		cout << "nfs_server : " << endl;
		const rapidjson::Value& nfsValue = document["nfs_server"];
		if(nfsValue.IsArray()){
			for(rapidjson::SizeType i = 0; i < nfsValue.Size(); ++i){
				cout << "    " << nfsValue[i].GetString() << endl;
			}
		}
	}

	if(document.HasMember("pl_name")){
		const rapidjson::Value& nfsValue = document["pl_name"];
		if(nfsValue.IsArray()){
			for(rapidjson::SizeType i = 0; i < nfsValue.Size(); ++i){
				_mManager.pl_name[i] = nfsValue[i].GetString();
			}
		}
	}

	if(document.HasMember("upLoadIndex")){
		const rapidjson::Value &uIVal = document["upLoadIndex"];
		if(uIVal.IsArray()){
			for(rapidjson::SizeType i = 0; i < uIVal.Size(); ++i){
				_mManager.upLoadIndex[i] = uIVal[i].GetInt();
			}
		}
	}

	if(document.HasMember("username")){
		const rapidjson::Value &usernameVal = document["username"];
		_mManager.username = usernameVal.GetString();
	}

	if(document.HasMember("password")){
		const rapidjson::Value &psdVal = document["password"];
		_mManager.password = psdVal.GetString();
	}

	if(document.HasMember("verCode")){
		const rapidjson::Value &cdVal = document["verCode"];
		_mManager.verCode = cdVal.GetInt();
	}

	if(document.HasMember("verString")){
		const rapidjson::Value &vsVal = document["verString"];
		_mManager.verString = vsVal.GetString();
	}

	if(document.HasMember("token")){
		const rapidjson::Value &tkVal = document["token"];
		_mManager.token = tkVal.GetString();
		_mManager.processStatus = DEVICE_REQUIRE_ACCESS_RIGHT;
	}else{
		_mManager.processStatus = DEVICE_LOGIN_PLATFORM;
	}

	if(document.HasMember("timestamp")){
		const rapidjson::Value &tsVal = document["timestamp"];
		_mManager.timestamp = tsVal.GetString();
	}
	
	return str;
}

bool UpdateConfigfile(const char *key,void *val,Json_Type jsontype)
{
	std::ifstream tmp(CONFIG_PATH);
  	std::string str((std::istreambuf_iterator<char>(tmp)),
                  std::istreambuf_iterator<char>());

	rapidjson::Document document;
  	document.Parse(str.c_str());

	if(jsontype != Type_ARRAY){
		if(document.HasMember(key)){
			document.RemoveMember(key);
		}
	}
	
	if(jsontype == Type_STRING){
		rapidjson::Value strValue(rapidjson::kStringType);
		strValue.SetString((const char *)val,document.GetAllocator());
		Value _key(key, document.GetAllocator());
		document.AddMember(_key,strValue,document.GetAllocator());
		printf("add string done\r\n");
	}else if(jsontype == Type_INT){
		int *numVal = (int*)val;
		rapidjson::Value numValue(rapidjson::kNumberType);
		numValue.SetInt((*numVal));
		Value _key(key, document.GetAllocator());
		document.AddMember(_key,numValue,document.GetAllocator());
		printf("add number done\r\n");
	}else if(jsontype == Type_ARRAY){
		int *numVal = (int*)val;
		rapidjson::Value &uIVal = document["upLoadIndex"];
		for(rapidjson::SizeType i = 0; i < uIVal.Size(); ++i){
			uIVal[i].SetInt(numVal[i]);
		}
		printf("modify array done\r\n");
	}
	
	rapidjson::StringBuffer buffer;
	rapidjson::Writer< rapidjson::StringBuffer > writer(buffer);
	document.Accept(writer);

	const char* updatestr = buffer.GetString();
	FILE * pFile = fopen (CONFIG_PATH , "w");
	if (!pFile) return false;
	fwrite(updatestr,sizeof(char),strlen(updatestr),pFile);
	fclose(pFile);
	system("sync");

	return true;
}


void *myOTA_run(void *para){

	_mManager.todayDate = my_ferrero.get_nowtime();
	while(1){		
		string nowDate = my_ferrero.get_nowtime();
		if(nowDate.compare(_mManager.todayDate) == 0){
			sleep(10);
			printf("%s thread heat beat\r\n",__func__);
		}else{
			_mManager.todayDate = nowDate;
			UpdateConfigfile("timestamp",(void *)nowDate.c_str(),Type_STRING);
			if(_mManager.upLoadIndex[0] != 1){
				_mManager.upLoadIndex[0] = 1;
				_mManager.upLoadIndex[1] = 1;
				_mManager.upLoadIndex[2] = 1;
				UpdateConfigfile("upLoadIndex",(void *)&_mManager.upLoadIndex,Type_ARRAY);
			}

			if(_mManager.processStatus == DEVICE_PROCESS_READY){
				string code_str = to_string(_mManager.verCode);
				string upDate_url = getupdate + "lastVersionCode=" + code_str + "&lastVersionName=" + _mManager.verString;
				printf("\r\nupDate_url :\r\n%s \r\n",upDate_url.c_str());

				string Response;
				std::list<std::string> slist{("Content-Type:application/json;charset=UTF-8")};
				string authorization = "Authorization:" + _mManager.token;
				slist.push_back(authorization);

				CURLcode code = curl.curl_get_req(upDate_url,Response,slist,false,10,10);
				cout << "code:" << code << endl;
				cout << "Response:" << Response << endl;

				rapidjson::Document resPon;
				resPon.Parse(Response.c_str());

				if((code != CURLE_OK)||resPon.HasParseError()){
					goto err2;
				}

				if(resPon.HasMember("versionCode")){
					const rapidjson::Value &versionCode = resPon["versionCode"];
					int lasted_Code = versionCode.GetInt();
					if(lasted_Code > _mManager.verCode){
						printf("find new version\r\n");

						if(resPon.HasMember("versionUrl")){
							const rapidjson::Value &versionUrl = resPon["versionUrl"];
							_mManager.otaurl = versionUrl.GetString();
							printf("otaurl = %s\r\n",_mManager.otaurl.c_str());

							if(resPon.HasMember("md5Sum")){
								_mManager.md5val.clear();
								const rapidjson::Value &MD5 = resPon["md5Sum"];
								_mManager.md5val = MD5.GetString();
								printf("md5val = %s\r\n",_mManager.md5val.c_str());
							}

							sleep(1);
							curl.Download(_mManager.otaurl,downLoad_file);
							curl.DownloadFinish();
							printf("DownLoad over...\r\n");

							if(access(downLoad_file, F_OK ) != -1 ){
								char *md5_val = (char *)malloc(32*sizeof(char *));
								Compute_file_md5(downLoad_file,md5_val);
								string md5_download_file = md5_val;
								printf("download file md5 val = [%s]\r\n",md5_download_file.c_str());
								if(md5_download_file.compare(_mManager.md5val) == 0){
									printf("ota file down load success\r\n");
									_mManager.processStatus = DEVICE_OTA_PROCESS;
								}else{
									printf("ota file down load failed,repeat again \r\n");
									sleep(5);
								}
								free(md5_val);
							} 
						}else if(resPon.HasMember("code")&&resPon.HasMember("message")){
							_mManager.processStatus = DEVICE_LOGIN_PLATFORM;
						}else{
							sleep(5);
						}
					}else{
						printf("\r\n********************no version found*********************\r\n");
					}
				}else if(resPon.HasMember("code")&&resPon.HasMember("message")){	//Response:{"code":"401","message":"Need authorization."}
					_mManager.processStatus = DEVICE_LOGIN_PLATFORM;
				}
			}else if(_mManager.processStatus == DEVICE_OTA_PROCESS){
				//执行ota脚本
			}
		}
err2:
		sleep(2);

	}
}


void *uploadLog_run(void *para){
	_mManager.processStatus = DEVICE_LOGIN_PLATFORM;
	string ConfigData = ReadConfigfile(CONFIG_PATH);
	if(ConfigData.empty()){
		char cp_cmd[256];
		memset(cp_cmd,0x0,sizeof(cp_cmd));
		sprintf(cp_cmd,"cp %s %s",CONFIG_PATH_BACKUP,CONFIG_PATH);
		system(cp_cmd);
		system("sync");
		exit(0);
	}
	
	printf("ConfigData = %s\r\n",ConfigData.c_str());
	string now_tmp = my_ferrero.get_nowtime();
	if(!_mManager.timestamp.empty()){
		if(now_tmp.compare(_mManager.timestamp) != 0){
			UpdateConfigfile("timestamp",(void *)now_tmp.c_str(),Type_STRING);
			if(_mManager.upLoadIndex[0] != 1){
				_mManager.upLoadIndex[0] = 1;
				UpdateConfigfile("upLoadIndex",(void *)&_mManager.upLoadIndex,Type_ARRAY);
			}
		}
	}


	while(1){
		if(_mManager.processStatus == DEVICE_LOGIN_PLATFORM){		//
			if((_mManager.username.length() > 0)&&(_mManager.password.length() > 0)){
				StringBuffer login_str;
				Writer<StringBuffer> writer(login_str);
				writer.StartObject();
				writer.Key("password");
				writer.String(_mManager.password.c_str());
				writer.Key("username");
				writer.String(_mManager.username.c_str());
				writer.EndObject();
				string jsonContext = login_str.GetString();
				printf("%s\r\n",jsonContext.c_str());

				_mManager.token.clear();
				std::list<std::string> slist{("Content-Type:application/json;charset=UTF-8")};
				CURLcode code = curl.curl_post_req(getlogintoken,jsonContext, _mManager.token, slist, false, 10, 10);
				cout << "code:" << code << endl;
				cout << "_mManager.token:" << _mManager.token << endl;

				if(_mManager.token.length()){
					UpdateConfigfile("token",(void *)_mManager.token.c_str(),Type_STRING);
					_mManager.processStatus = DEVICE_REQUIRE_ACCESS_RIGHT;
				}else{
					sleep(5);
				}
			}else{
				printf("Invalid user information \r\n");
				exit(0);
			}
		}else if(_mManager.processStatus == DEVICE_REQUIRE_ACCESS_RIGHT){
#if 0	//获取访问权限
			string Response;
			Response.clear();
			std::list<std::string> slist{("Content-Type:application/json;charset=UTF-8")};
			string authorization = "Authorization:" + _mManager.token;
			slist.push_back(authorization);

			CURLcode code = curl.curl_get_req(getaccessright,Response,slist,false,10,10);
			cout << "code:" << code << endl;
			cout << "Response:" << Response << endl;

			if(Response.length() > 0){
				rapidjson::Document resPon;
				resPon.Parse(Response.c_str());
				if(resPon.HasMember("roles")){
					_mManager.processStatus = DEVICE_PROCESS_READY;
				}else if(resPon.HasMember("code")&&resPon.HasMember("message")){
					_mManager.processStatus = DEVICE_LOGIN_PLATFORM;
				}else{
					sleep(5);
				}
			}
#endif
			//开机查询一次最新版本
			string code_str = to_string(_mManager.verCode);
			string upDate_url = getupdate + "lastVersionCode=" + code_str + "&lastVersionName=" + _mManager.verString;
			printf("\r\nupDate_url :\r\n%s \r\n",upDate_url.c_str());

			string Response;
			std::list<std::string> slist{("Content-Type:application/json;charset=UTF-8")};
			string authorization = "Authorization:" + _mManager.token;
			slist.push_back(authorization);

			CURLcode code = curl.curl_get_req(upDate_url,Response,slist,false,10,10);
			cout << "code:" << code << endl;
			cout << "Response:" << Response << endl;

			rapidjson::Document resPon;
			resPon.Parse(Response.c_str());

			if((code != CURLE_OK)||resPon.HasParseError()){
				goto err1;
			}


			if(resPon.HasMember("versionCode")){
				const rapidjson::Value &versionCode = resPon["versionCode"];
				int lasted_Code = versionCode.GetInt();
				if(lasted_Code > _mManager.verCode){
					printf("find new version\r\n");

					if(resPon.HasMember("versionUrl")){
						const rapidjson::Value &versionUrl = resPon["versionUrl"];
						_mManager.otaurl = versionUrl.GetString();
						printf("otaurl = %s\r\n",_mManager.otaurl.c_str());

						if(resPon.HasMember("md5Sum")){
							_mManager.md5val.clear();
							const rapidjson::Value &MD5 = resPon["md5Sum"];
							_mManager.md5val = MD5.GetString();
							printf("md5val = %s\r\n",_mManager.md5val.c_str());
						}

						sleep(1);
						curl.Download(_mManager.otaurl,downLoad_file);
						curl.DownloadFinish();
						printf("DownLoad over...\r\n");

						if(access(downLoad_file, F_OK ) != -1 ){
							char *md5_val = (char *)malloc(32*sizeof(char *));
							Compute_file_md5(downLoad_file,md5_val);
							string md5_download_file = md5_val;
							printf("download file md5 val = [%s]\r\n",md5_download_file.c_str());
							if(md5_download_file.compare(_mManager.md5val) == 0){
								printf("ota file down load success\r\n");
								_mManager.processStatus = DEVICE_OTA_PROCESS;
							}else{
								printf("ota file down load failed,repeat again \r\n");
								sleep(5);
							}
							free(md5_val);
						} 
					}else{
						sleep(5);
					}
				}else{
					_mManager.processStatus = DEVICE_PROCESS_READY;
					printf("\r\n********************no version found*********************\r\n");
					sleep(2);
					GetTimeOf_Day(&_mManager.upLoadTime);
					
				}
			}else if(resPon.HasMember("code")&&resPon.HasMember("message")){	//Response:{"code":"401","message":"Need authorization."}
				_mManager.processStatus = DEVICE_LOGIN_PLATFORM;
			}
		}else if(_mManager.processStatus == DEVICE_PROCESS_READY){
			struct hal_timeval now;
			struct hal_timeval tmp_tv;
			GetTimeOf_Day(&now);

			tmp_tv.tv_sec = now.tv_sec - _mManager.upLoadTime.tv_sec;
			if(tmp_tv.tv_sec > 10){
				GetTimeOf_Day(&_mManager.upLoadTime);

				string upload_Index = to_string(_mManager.upLoadIndex[0]);
				string now_d = my_ferrero.get_nowtime();
				string up_loadFile = NFS_PATH + upload_Index + _mManager.pl_name[0] + now_d + ".tar.gz";
				if(access(up_loadFile.c_str(), F_OK ) != -1 ){
					printf("find file \r\n");
					long http_res = curl.Upload(uploadurl,up_loadFile);
					if(http_res == 200){
						_mManager.upLoadIndex[0] ++;
						UpdateConfigfile("upLoadIndex",(void *)&_mManager.upLoadIndex,Type_ARRAY);
					}
				}else{
					printf("no file \r\n");
				}
			}

			

		}
err1:
		sleep(2);
	}
}



int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_ALL);

	pthread_t id_o;
    pthread_create(&id_o, NULL, myOTA_run, NULL);

	pthread_t id_k;
    pthread_create(&id_k, NULL, uploadLog_run, NULL);

    while (1) {
		sleep(10);
    }

    curl_global_cleanup();
    return 0;
}
