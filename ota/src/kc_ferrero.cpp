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
#include "kc_ferrero.hpp"

using namespace std;

namespace kc_ferrero{

kc_ferrero::kc_ferrero(void)
{
    cout << "my_curl Object is being created" << endl;
}

std::string kc_ferrero::get_nowtime(void)
{
    time_t t = time(0); 
    string tim;
    char tmp[64]; 
    strftime( tmp, sizeof(tmp), "%Y%m%d%M",localtime(&t) ); 
    tim = tmp;
    printf("%s\r\n",tim.c_str());

    return tim; 
}


}