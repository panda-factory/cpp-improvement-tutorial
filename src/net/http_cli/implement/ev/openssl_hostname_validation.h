//
// Created by admin on 2021/2/1.
//

#ifndef TEST_OPENSSL_HOSTNAME_VALIDATION_H
#define TEST_OPENSSL_HOSTNAME_VALIDATION_H

#include <openssl/x509v3.h>
#include <openssl/ssl.h>

enum class HostnameValidationResult : uint8_t {
    MatchFound,
    MatchNotFound,
    NoSANPresent,
    MalformedCertificate,
    Error
};

HostnameValidationResult ValidateHostname(const char *hostname, const X509 *server_cert);


#endif //TEST_OPENSSL_HOSTNAME_VALIDATION_H
