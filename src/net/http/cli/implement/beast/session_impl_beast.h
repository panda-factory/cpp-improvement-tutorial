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

#include "net/http_cli/response.h"
#include "net/http_cli/session_impl.h"
#include "net/http_cli/error.h"

namespace http::cli {
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class SessionImplBeast : public SessionImpl, public std::enable_shared_from_this<SessionImplBeast> {
public:
    using StreamOpaque = std::variant<std::monostate, tcp::socket, ssl::stream<tcp::socket>>;

    void SetUrl(const Url &url) override;

    void SetParameters(const Parameters &parameters) override;

    void SetParameters(Parameters &&parameters) override;

    void SetHandler(const ResponseHandler&& onResponse) override;

    void SetHeader(const Header &header) override;

    void SetTimeout(const Timeout &timeout) override;

    void SetAuth(const authentication &auth) override;

    void SetDigest(const Digest &auth) override;

    void SetMethod(const Method& method) override;

    void SetMultipart(Multipart &&multipart) override;

    void SetMultipart(const Multipart &multipart) override;

    void SetRedirect(const bool &redirect) override;

    void SetMaxRedirects(const MaxRedirects &max_redirects) override;

    void SetCookies(const Cookies &cookies) override;

    void SetBody(body &&body) override;

    void SetBody(const body &body) override;

    void DoRequest() override;
    SessionImplBeast();
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
    boost::asio::io_context ioc_;
    boost::asio::steady_timer timeouter{ioc_};
    boost::asio::steady_timer timeouterForGracefulShutdown{ioc_};
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
    ResponseHandler responseHandler_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
};
} //namespace http::cli


#endif //TEST_SESSIONIMPLBEAST_H
