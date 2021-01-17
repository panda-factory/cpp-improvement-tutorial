//
// Created by admin on 2021/1/10.
//

#ifndef TEST_ERROR_H
#define TEST_ERROR_H
#include <string>
namespace HttpCli {
enum class ErrorCode : unsigned int {
    OK = 0,
    TIMEDOUT,
    CONNECTION_FAILURE,
    EMPTY_RESPONSE,
    HOST_RESOLUTION_FAILURE,
    INTERNAL_ERROR,
    INVALID_URL_FORMAT,
    NETWORK_RECEIVE_ERROR,
    NETWORK_SEND_FAILURE,
    PROXY_RESOLUTION_FAILURE,
    SSL_CONNECT_ERROR,
    SSL_LOCAL_CERTIFICATE_ERROR,
    SSL_REMOTE_CERTIFICATE_ERROR,
    SSL_CACERT_ERROR,
    GENERIC_SSL_ERROR,
    UNSUPPORTED_PROTOCOL,
    UNKNOWN_ERROR = 1000,

};

class Error {
public:
    Error();
    Error(ErrorCode code);

    explicit operator bool() const {
        return code_ != ErrorCode::OK;
    }

    ErrorCode code_;

    //! Converts Error to human readable std::string
    operator std::string() const {
        std::stringstream ss;
        ss << "error:" << static_cast<unsigned int>(this->code_);
        return ss.str();
    }
};

std::ostream& operator<<(std::ostream& os, const Error& err) {
    os << std::string(err);
    return os;
}

} // namespace HttpCli


#endif //TEST_ERROR_H
