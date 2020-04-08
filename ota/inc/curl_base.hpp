#ifndef _CURL_BASE_HPP_
#define _CURL_BASE_HPP_

#include <string>

namespace CURL_BASE{
class curl_base{
public:
	virtual CURLcode curl_get_req(const std::string &url, std::string &response,std::list<std::string> listRequestHeader,
							bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout);

	virtual CURLcode curl_post_req(const std::string &url, const std::string &postParams, std::string &response, std::list<std::string> listRequestHeader, 
								bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout);

	curl_base();

private:
	 void private_post_print(void);
	};

}


#endif
