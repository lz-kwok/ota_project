#ifndef _BD_CURL_HPP_
#define _BD_CURL_HPP_

#include <string>
#include "curl_base.hpp"

namespace mycurl{
class my_curl
	:public CURL_BASE::curl_base{
public:
	CURLcode curl_get_req(const std::string &url, std::string &response,std::list<std::string> listRequestHeader,
							bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout) override;

	CURLcode curl_post_req(const std::string &url, const std::string &postParams, std::string &response, std::list<std::string> listRequestHeader, 
								bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout) override;

	CURLcode CURL_Post_Req(const std::string &url, const std::string &postParams, std::string &response, std::list<std::string> listRequestHeader, 
							bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout);

	CURLcode Download(std::string strUrl,std::string filepath);

	my_curl();
//private:
	
	};

}


#endif
