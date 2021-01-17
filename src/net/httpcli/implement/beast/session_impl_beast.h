//
// Created by admin on 2021/1/10.
//

#ifndef TEST_SESSIONIMPLBEAST_H
#define TEST_SESSION_IMPL_BEAST_H

#include <functional>
#include <fstream>
#include <regex>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <utility>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "net/httpcli/response.h"
#include "net/httpcli/session.h"
#include "net/httpcli/error.h"

namespace HttpCli {
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class SessionImplBeast : public Session, public std::enable_shared_from_this<SessionImplBeast> {
public:
    using StreamOpaque = std::variant<std::monostate, tcp::socket, ssl::stream<tcp::socket>>;
    void DoRequest();

private:
    void DoOneRequest();
    tcp::socket& GetPlainStream();
    ssl::stream<tcp::socket>& GetTlsStream();
    void HandleFail(boost::system::error_code ec, ErrorCode http_ec);
    bool IsTlsStream();
    void OnConnect(boost::beast::error_code ec);
    void OnFail(boost::system::error_code ec, ErrorCode http_ec);
    void OnHandShake(boost::system::error_code ec);
    void OnRead( boost::system::error_code ec, std::size_t bytes_transferred);
    void OnResolve( boost::system::error_code ec, tcp::resolver::results_type results);
    void OnShutdown(boost::system::error_code ec);
    void OnTimeout(const boost::system::error_code& ec);
    void OnTooLongAsyncShutdown(boost::system::error_code ec);
    void OnWrite( boost::system::error_code ec, std::size_t bytes_transferred);
    void PrepareRequest();

    std::chrono::milliseconds timeout_ = std::chrono::milliseconds{0};
    boost::asio::steady_timer timeouter{ioc_};
    boost::asio::steady_timer timeouterForGracefulShutdown{ioc_};
    boost::asio::io_context ioc_;
    bool redirect_ = true;
    bool followNextRedirect = false;
    std::int32_t numberOfRedirects = std::numeric_limits<std::int32_t>::max();
    http::request<http::string_body> request_;
    http::verb method_;
    Url url_;
    std::function<void(Response&&)> onResponse;
    Parameters parameters_;
    std::shared_ptr<boost::beast::http::response_parser<boost::beast::http::string_body>> responseParser_;
    StreamOpaque stream_;
    ssl::context context_{ssl::context::tls_client};
    tcp::resolver resolver_{ioc_};
    std::function<void(Response&&)> responseHandler_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
};
} //namespace HttpCli


#endif //TEST_SESSIONIMPLBEAST_H