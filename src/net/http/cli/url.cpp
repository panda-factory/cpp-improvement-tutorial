//
// Created by admin on 2021/1/10.
//

#include "url.h"

namespace http::cli {

Url::Url(const std::string& url) : url(url){
    ParseUrl();
}


bool Url::IsHttps() const {
    return protocol == "https";
}
void Url::ParseUrl() {

    return;
}
} //namespace http::cli