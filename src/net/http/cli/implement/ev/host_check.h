//
// Created by admin on 2021/2/1.
//

#ifndef TEST_HOST_CHECK_H
#define TEST_HOST_CHECK_H


#define CURL_HOST_NOMATCH 0
#define CURL_HOST_MATCH   1
int Curl_cert_hostcheck(const char *match_pattern, const char *hostname);


#endif //TEST_HOST_CHECK_H
