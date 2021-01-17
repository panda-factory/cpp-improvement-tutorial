//
// Created by admin on 2021/1/10.
//

#include "session_impl_beast.h"

namespace HttpCli {

void SessionImplBeast::DoRequest() {
    do {
        ioc_.reset();
        DoOneRequest();
        ioc_.run();
    } while (followNextRedirect);
}
void SessionImplBeast::DoOneRequest() {

    // Cleanup for subsequent calls
    followNextRedirect = false;
    responseParser_ = std::make_shared<boost::beast::http::response_parser<boost::beast::http::string_body>>();

    // We need to download whatever fits in RAM
    responseParser_->body_limit(std::numeric_limits<std::uint64_t>::max());
    //TODO: change the limit for uploading too

    request_.method(method_);

    request_.version(11);
    std::stringstream target; target << url_.path;
    if ( !url_.parameters.empty() || !parameters_.content.empty() ) {
        target << "?" << url_.parameters << parameters_.content;
    }
    request_.target(target.str());
    request_.set(http::field::host, url_.host);
    request_.set(http::field::user_agent, "xxhr/v0.0.1"); //TODO: add a way to override from user and make a version macro
    request_.prepare_payload(); // Compute Content-Length and related headers

    if (url_.https()) {
        stream_.emplace<ssl::stream<tcp::socket>>(ioc_, context_);
    } else {
        stream_.emplace<tcp::socket>(ioc_);
    }

    PrepareRequest();

    if (timeout_ != std::chrono::milliseconds(0)) {
        timeouter.expires_after(timeout_);
            timeouter.async_wait(std::bind(&SessionImplBeast::OnTimeout, this, std::placeholders::_1));
    }

}

tcp::socket& SessionImplBeast::GetPlainStream() {
    return std::get<tcp::socket>(stream_);
}

ssl::stream<tcp::socket>& SessionImplBeast::GetTlsStream() {
    return std::get<ssl::stream<tcp::socket>>(stream_);
}

void SessionImplBeast::HandleFail(boost::system::error_code ec, ErrorCode http_ec) {
    //TODO: if (trace)
    std::cerr << ec << ": " << ec.message() << " distilled into : " << uint32_t(http_ec) << "\n";

    responseHandler_(Response(
            0, // 0 for errors which are on the layer belows http, like XmlHttpRequest.
            Error{http_ec},
            std::string{},
            Header{},
            url_,
            Cookies{}
    ));
}

bool SessionImplBeast::IsTlsStream() {
    return std::holds_alternative<ssl::stream < tcp::socket>>(stream_);
}

void SessionImplBeast::OnConnect(boost::system::error_code ec) {
    if(ec)
        return OnFail(ec, ErrorCode::CONNECTION_FAILURE);

    if (IsTlsStream()) {
        // Perform the SSL handshake
        auto& stream = GetTlsStream();
        stream.async_handshake(
                ssl::stream_base::client,
                std::bind(
                        &SessionImplBeast::OnHandShake,
                        shared_from_this(),
                        std::placeholders::_1));
    } else {
        // Plain HTTP
        // consider handshake was performed.
        OnHandShake(ec);
    }
}

void SessionImplBeast::OnFail(boost::system::error_code ec, ErrorCode http_ec) {
    //TODO: if (trace)
    std::cerr << ec << ": " << ec.message() << " distilled into : " << uint32_t(http_ec) << "\n";

    responseHandler_(Response(
            0, // 0 for errors which are on the layer belows http, like XmlHttpRequest.
            Error{http_ec},
            std::string{},
            Header{},
            url_,
            Cookies{}
    ));
}

void SessionImplBeast::OnRead( boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    timeouter.cancel();

    if(ec)
        return OnFail(ec, ErrorCode::NETWORK_RECEIVE_ERROR);

    http::response<http::string_body> res = responseParser_->get();

    // Write the message to standard out
    Header responseHeaders;
    Cookies response_cookies;
    for (auto&& header : res.base()) {
        if (header.name() == http::field::set_cookie) { // TODO: case insensitive
            response_cookies
                    .ParseCookieString(std::string(header.value()));
        } else {
            responseHeaders
                    .insert_or_assign(
                            std::string(header.name_string()),
                            std::string(header.value()));
        }
    }

    // Gracefully close the stream
    if (IsTlsStream()) {

        // Give a bit of time for graceful shutdown but otherwise just forcefully close
        timeouterForGracefulShutdown.expires_after(std::chrono::milliseconds(500));
        timeouterForGracefulShutdown.async_wait(
                std::bind(
                        &SessionImplBeast::OnTooLongAsyncShutdown,
                        shared_from_this(),
                        std::placeholders::_1)
        );

        auto& stream = GetTlsStream();
        stream.async_shutdown(
                std::bind(
                        &SessionImplBeast::OnShutdown,
                        shared_from_this(),
                        std::placeholders::_1));

    } else {
        OnShutdown(ec);
    }


    if (responseHeaders.find("Location") != responseHeaders.end()) {
        SetUrl(responseHeaders["Location"]);
        if ( (redirect_) && (numberOfRedirects > 0) )  {
            --numberOfRedirects;
            followNextRedirect = true; // Follow the redirection
        }
    } else {

        onResponse(Response(
                res.result_int(),
                Error{},
                res.body(),
                responseHeaders,
                url_,
                response_cookies
        ));

    }
}

void SessionImplBeast::OnResolve( boost::system::error_code ec, tcp::resolver::results_type results) {
    if(ec)
        return OnFail(ec, ErrorCode::HOST_RESOLUTION_FAILURE);

    // Make the connection on the IP address we get from a lookup
    tcp::socket* socket;
    if (IsTlsStream()) {
        socket = &GetTlsStream().next_layer();
    } else {
        socket = &GetPlainStream();
    }

    boost::asio::async_connect(
            *socket,
            results.begin(),
            results.end(),
            std::bind(
                    &SessionImplBeast::OnConnect,
                    shared_from_this(),
                    std::placeholders::_1));
}

void SessionImplBeast::OnShutdown(boost::system::error_code ec) {
    timeouterForGracefulShutdown.cancel();
    //if(ec == boost::asio::error::eof) {
    // Rationale:
    // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
    ec.assign(0, ec.category());
    //}
    if(ec)
        return OnFail(ec, ErrorCode::GENERIC_SSL_ERROR);

    // If we get here then the connection is closed gracefully
}

void SessionImplBeast::OnHandShake(boost::beast::error_code ec) {
    if(ec)
        return OnFail(ec, ErrorCode::SSL_CONNECT_ERROR);

    // Send the HTTP request to the remote host
    std::visit([this](auto& stream) {
        if constexpr (std::is_same_v<std::monostate,std::decay_t<decltype(stream)>>) return;
        else {
            http::async_write(stream, request_,
                              std::bind(
                                      &SessionImplBeast::OnWrite,
                                      shared_from_this(),
                                      std::placeholders::_1,
                                      std::placeholders::_2));
        }
    }, stream_);
}

void SessionImplBeast::OnTimeout(const boost::system::error_code& ec) {
    if (ec != boost::asio::error::operation_aborted) {

        ioc_.stop();

        tcp::socket* socket;
        if (IsTlsStream()) {
            socket = &GetTlsStream().next_layer();
        } else {
            socket = &GetPlainStream();
        }

        boost::system::error_code ec_dontthrow;
        socket->cancel(ec_dontthrow);
        socket->close(ec_dontthrow);

        OnFail(ec, ErrorCode::TIMEDOUT);
    }
}

void SessionImplBeast::OnTooLongAsyncShutdown(boost::system::error_code ec) {
    if (ec != boost::asio::error::operation_aborted) {

        ioc_.stop();

        tcp::socket* socket;
        if (IsTlsStream()) {
            socket = &GetTlsStream().next_layer();
        } else {
            socket = &GetPlainStream();
        }

        boost::system::error_code ec_dontthrow;
        socket->cancel(ec_dontthrow);
        socket->close(ec_dontthrow);
    }
}
void SessionImplBeast::OnWrite( boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return OnFail(ec, ErrorCode::NETWORK_SEND_FAILURE);

    // Receive the HTTP response
    std::visit([this](auto& stream) {
        if constexpr (std::is_same_v<std::monostate,std::decay_t<decltype(stream)>>) return;
        else {
            http::async_read(stream, buffer_, *responseParser_,
                             std::bind(
                                     &SessionImplBeast::OnRead,
                                     shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));

        }
    }, stream_);
}

void SessionImplBeast::PrepareRequest() {

    if (IsTlsStream()) {

        auto& stream = GetTlsStream();

        // Set SNI Hostname (many hosts need this to handshake successfully)
        // XXX: openssl specificae, abstract this shit please
        if(!SSL_set_tlsext_host_name(stream.native_handle(), url_.host.data()))
        {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }
    }

    // Look up the domain name
    resolver_.async_resolve(
            url_.host.data(),
            url_.port.data(),
            std::bind(
                    &SessionImplBeast::OnResolve,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2));
}

} //namespace HttpCli