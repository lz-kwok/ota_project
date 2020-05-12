#ifndef _CURL_BASE_HPP_
#define _CURL_BASE_HPP_

#include <string>
#include <functional>


struct MemoryStruct
{
    char *memory;
    size_t size;
};
using Callback = std::function<void(const MemoryStruct &m)>;


namespace CURL_BASE{
class curl_base{
public:
	virtual CURLcode curl_get_req(const std::string &url, std::string &response,std::list<std::string> listRequestHeader,
							bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout);

	virtual CURLcode curl_post_req(const std::string &url, const std::string &postParams, std::string &response, std::list<std::string> listRequestHeader, 
								bool bResponseIsWithHeaderData , int nConnectTimeout, int nTimeout);

	virtual CURLcode Download(std::string strUrl,std::string filepath);

	virtual void DownloadFinish();

	virtual CURLcode Upload(std::string strUrl,std::string filepath);

	curl_base();

private:
	void private_post_print(void);
	CURL *curl_handle;
	MemoryStruct chunk;
	Callback callback;
	};

}


#endif
