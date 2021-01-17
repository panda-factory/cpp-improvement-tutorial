//
// Created by admin on 2021/1/10.
//

#include "error.h"

namespace HttpCli {
Error::Error() : code_{ErrorCode::OK} {

}
Error::Error(ErrorCode code) : code_{code} {}
} // namespace HttpCli