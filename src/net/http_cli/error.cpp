//
// Created by admin on 2021/1/10.
//

#include "error.h"

namespace http {
Error::Error() : code_{ErrorCode::OK} {

}
Error::Error(ErrorCode code) : code_{code} {

}

bool Error::operator== (const ErrorCode& code) const {
    return code_ == code;
}

Error::operator std::string() const {
    std::stringstream ss;
    ss << "error:" << static_cast<unsigned int>(this->code_);
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Error& err) {
    os << std::string(err);
    return os;
}
} // namespace http