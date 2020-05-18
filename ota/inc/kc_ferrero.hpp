#ifndef _BD_CURL_HPP_
#define _BD_CURL_HPP_

#include <string>
#include "curl_base.hpp"

namespace kc_ferrero{
class kc_ferrero
	:public CURL_BASE::curl_base{
public:
	kc_ferrero(void);
	std::string get_nowtime(void);

//private:
	
	};

}


#endif
