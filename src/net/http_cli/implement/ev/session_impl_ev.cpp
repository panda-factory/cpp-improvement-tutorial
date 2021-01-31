//
// Created by admin on 2021/1/21.
//

#include "session_impl_ev.h"

#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/http_struct.h>


#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "core/scope_exit.h"
#include "core/logging.h"
namespace HttpCli {
namespace {

void HandleResponse(struct evhttp_request* req, void* ctx) {
    char buffer[256];
    int nread;
    SessionImplEV* that = (SessionImplEV*)(ctx);
    CORE_CHECK(that);

    if (!req || !evhttp_request_get_response_code(req)) {
        /* If req is NULL, it means an error occurred, but
         * sadly we are mostly left guessing what the error
         * might have been.  We'll do our best... */
        struct bufferevent *bev = (struct bufferevent *) ctx;
        unsigned long oslerr;
        int printed_err = 0;
        int errcode = EVUTIL_SOCKET_ERROR();
        CORE_LOG(ERROR) << "some request failed - no idea which one though!";
        /* Print out the OpenSSL error queue that libevent
         * squirreled away for us, if any. */
        while ((oslerr = bufferevent_get_openssl_error(bev))) {
            ERR_error_string_n(oslerr, buffer, sizeof(buffer));
            CORE_LOG(ERROR) << buffer;
            printed_err = 1;
        }
        /* If the OpenSSL error queue was empty, maybe it was a
         * socket error; let's try printing that. */
        if (!printed_err) {
            std::ostringstream oss;
            oss << "socket error = " << evutil_socket_error_to_string(errcode)
                << ", " << errcode;
            that->OnResponse(Response(0, // 0 for errors which are on the layer belows http, like XmlHttpRequest.
                                      Error{ErrorCode::UNKNOWN_ERROR},
                                      oss.str(),
                                      Header{},
                                      that->url()));
        }
        return;
    }

    CORE_LOG(INFO) << "Response line: "
                   << evhttp_request_get_response_code(req) << " "
                   << evhttp_request_get_response_code_line(req);

    std::ostringstream oss;
    while ((nread = evbuffer_remove(evhttp_request_get_input_buffer(req),
                                    buffer, sizeof(buffer)))
           > 0) {
        /* These are just arbitrary chunks of 256 bytes.
         * They are not lines, so we can't treat them as such. */
        oss << buffer;
    }

    that->OnResponse(Response(evhttp_request_get_response_code(req),
                              Error{ErrorCode::OK},
                              oss.str(),
                              Header{},
                              that->url()));
}
void OnError(enum evhttp_request_error, void *ctx) {

    SessionImplEV* that = (SessionImplEV*)(ctx);
    CORE_CHECK(!that);
    that->OnResponse(Response(0,
                              Error{ErrorCode::OK},
                              "oss.str()",
                              Header{},
                              that->url()));
}
} // namespace
SessionImplEV::SessionImplEV() {
}

SessionImplEV::~SessionImplEV() {
}

#ifdef _WIN32
int SessionImplEV::AddCertForStore(X509_STORE *store, const std::string& name) {
    HCERTSTORE sys_store = NULL;
    PCCERT_CONTEXT ctx = NULL;
    int r = 0;

    sys_store = CertOpenSystemStore(0, name.c_str());
    if (!sys_store) {
        CORE_LOG(ERROR) << "failed to open system certificate store";
        return -1;
    }
    while ((ctx = CertEnumCertificatesInStore(sys_store, ctx))) {
        X509 *x509 = d2i_X509(NULL, (unsigned char const **)&ctx->pbCertEncoded,
                              ctx->cbCertEncoded);
        if (x509) {
            X509_STORE_add_cert(store, x509);
            X509_free(x509);
        } else {
            r = -1;
            HandleOpensslError("d2i_X509");
            break;
        }
    }
    CertCloseStore(sys_store, 0);
    return r;
}
#endif
bool ignoreCert = true;
int SessionImplEV::VerifyCertCallback(X509_STORE_CTX *x509_ctx, void *arg) {
    char cert_str[256];
    const char *host = (const char *) arg;
    const char *res_str = "X509_verify_cert failed";
    HostnameValidationResult res = Error;

    /* This is the function that OpenSSL would call if we hadn't called
     * SSL_CTX_set_cert_verify_callback().  Therefore, we are "wrapping"
     * the default functionality, rather than replacing it. */
    int ok_so_far = 0;

    X509 *server_cert = NULL;

    if (ignoreCert) {
        return 1;
    }

    ok_so_far = X509_verify_cert(x509_ctx);

    server_cert = X509_STORE_CTX_get_current_cert(x509_ctx);

    if (ok_so_far) {
        //res = validate_hostname(host, server_cert);

        switch (res) {
            case MatchFound:
                res_str = "MatchFound";
                break;
            case MatchNotFound:
                res_str = "MatchNotFound";
                break;
            case NoSANPresent:
                res_str = "NoSANPresent";
                break;
            case MalformedCertificate:
                res_str = "MalformedCertificate";
                break;
            case Error:
                res_str = "Error";
                break;
            default:
                res_str = "WTF!";
                break;
        }
    }

    X509_NAME_oneline(X509_get_subject_name (server_cert),
                      cert_str, sizeof (cert_str));

    if (res == MatchFound) {
        printf("https server '%s' has this certificate, "
               "which looks good to me:\n%s\n",
               host, cert_str);
        return 1;
    } else {
        printf("Got '%s' for hostname '%s' and certificate:\n%s\n",
               res_str, host, cert_str);
        return 0;
    }
}

void SessionImplEV::InitEventConnect() {
    int result = -1;
    if (!url_.IsHttps()) {
        evBuffer_ = bufferevent_socket_new(evBase_, -1, BEV_OPT_CLOSE_ON_FREE);
    } else {

#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x20700000L)
        // Initialize OpenSSL
	    SSL_library_init();
	    ERR_load_crypto_strings();
	    SSL_load_error_strings();
	    OpenSSL_add_all_algorithms();
#endif
        /* This isn't strictly necessary... OpenSSL performs RAND_poll
         * automatically on first use of random number generator. */
        result = RAND_poll();
        if (result == 0) {
            HandleOpensslError("RAND_poll");
            //goto error;
        }

        /* Create a new OpenSSL context */
        sslContext_ = SSL_CTX_new(SSLv23_method());
        if (!sslContext_) {
            HandleOpensslError("SSL_CTX_new");
            //goto error;
        }

        X509_STORE *store;
        /* Attempt to use the system's trusted root certificates. */
        store = SSL_CTX_get_cert_store(sslContext_);
#ifdef _WIN32
        if (AddCertForStore(store, "CA") < 0 ||
            AddCertForStore(store, "AuthRoot") < 0 ||
            AddCertForStore(store, "ROOT") < 0) {
            //goto error;
        }
#else // _WIN32
        if (X509_STORE_set_default_paths(store) != 1) {
			err_openssl("X509_STORE_set_default_paths");
			goto error;
		}
#endif // _WIN32

        SSL_CTX_set_verify(sslContext_, SSL_VERIFY_PEER, NULL);
        SSL_CTX_set_cert_verify_callback(sslContext_, VerifyCertCallback,
                                         (void *)(url_.host.c_str()));
        // Create OpenSSL bufferevent and stack evhttp on top of it
        ssl_ = SSL_new(sslContext_);
        if (ssl_ == NULL) {
            HandleOpensslError("SSL_new()");
            //goto error;
        }

#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
        // Set hostname for SNI extension
        SSL_set_tlsext_host_name(ssl_, url_.host.c_str());
#endif
        evBuffer_ = bufferevent_openssl_socket_new(evBase_, -1, ssl_,
                                                   BUFFEREVENT_SSL_CONNECTING,
                                                   BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    }
    bufferevent_openssl_set_allow_dirty_shutdown(evBuffer_, 1);

    evConn_ = evhttp_connection_base_bufferevent_new(evBase_, NULL, evBuffer_,
                                                     url_.host.c_str(), url_.port);
    if (evConn_ == NULL) {
        CORE_LOG(ERROR) << "evhttp_connection_base_bufferevent_new() failed";
    }

    if (retries_ > 0) {
        evhttp_connection_set_retries(evConn_, retries_);
    }

    if (timeout_.ToSeconds() > 0) {
        evhttp_connection_set_timeout(evConn_, timeout_.ToSeconds());
    }
}

bool SessionImplEV::DoInit() {
#ifdef _WIN32
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        wVersionRequested = MAKEWORD(2, 2);

        err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0) {
            CORE_LOG(ERROR) << "WSAStartup failed with error: " << err;
        }
    }
#endif // _WIN32

    evBase_ = event_base_new();
    if (evBase_ == nullptr) {
        CORE_LOG(ERROR) << "event_base_new()";
    }

    evReq_ = evhttp_request_new(HandleResponse, this);
    if (evReq_ == nullptr) {
        CORE_LOG(ERROR) << "evhttp_request_new fail";
    }
    evhttp_request_set_error_cb(evReq_, OnError);
    return true;
}

void SessionImplEV::DoRequest() {
    CORE_CHECK(!url_.url.empty()) << "url cannot be empty!";
    InitEventConnect();

    evhttp_add_header(evReq_->output_headers, "Host", url_.host.c_str());
    evhttp_add_header(evReq_->output_headers, "Connection", "close");

    int result = evhttp_make_request(evConn_, evReq_, EVHTTP_REQ_GET, url_.path.c_str());
    if (result != 0) {
        CORE_LOG(ERROR) << "evhttp_make_request() failed\n";
        //goto error;
    }

    event_base_dispatch(evBase_);
}

void SessionImplEV::HandleOpensslError(const std::string& func) {
    CORE_LOG(ERROR) << func << " failed:";

    /* This is the OpenSSL function that prints the contents of the
     * error stack to the specified file handle. */
    ERR_print_errors_fp (stderr);
}

void SessionImplEV::OnResponse(Response&& response) {
    responseHandler_(std::move(response));
}

void SessionImplEV::SetUrl(const Url &url) {
    struct evhttp_uri *httpUri = evhttp_uri_parse(url.url.c_str());
    SCOPE_GUARD{
        if (httpUri) {
            evhttp_uri_free(httpUri);
        }
    };

    url_ = url;
    url_.protocol = evhttp_uri_get_scheme(httpUri);
    url_.host = evhttp_uri_get_host(httpUri);
    if (auto port = evhttp_uri_get_port(httpUri) == -1) {
        url_.port = url_.IsHttps() ? 443 : 80;
    }
    if (auto path = evhttp_uri_get_path(httpUri)) {
        url_.path = std::string(path).size() == 0 ? "/" : std::string(path);
    }

    if (auto fragment = evhttp_uri_get_query(httpUri) != nullptr) {
        url_.fragment = fragment;
    }

}

void SessionImplEV::SetParameters(const Parameters &parameters) {

}

void SessionImplEV::SetParameters(Parameters &&parameters) {

}

void SessionImplEV::SetHandler(const ResponseHandler&& onResponse) {
    responseHandler_ = std::move(onResponse);
}

void SessionImplEV::SetHeader(const Header &headers) {
    std::string errmsg;

    struct evkeyvalq *output_headers = evhttp_request_get_output_headers(evReq_);
    for (const auto& header : headers) {
        if (evhttp_add_header(
                output_headers, header.first.c_str(), header.second.c_str())) {
            errmsg = "evhttp_add_header failed";
        }
    }
}

void SessionImplEV::SetTimeout(const Timeout &timeout) {
    timeout_ = timeout;
}

void SessionImplEV::SetAuth(const authentication &auth) {

}

void SessionImplEV::SetDigest(const Digest &auth) {

}

void SessionImplEV::SetMethod(const Method &method) {

}

void SessionImplEV::SetMultipart(Multipart &&multipart) {

}

void SessionImplEV::SetMultipart(const Multipart &multipart) {

}

void SessionImplEV::SetRedirect(const bool &redirect) {

}

void SessionImplEV::SetRetries(const Retries &retries) {
    retries_ = retries;
}

void SessionImplEV::SetMaxRedirects(const MaxRedirects &max_redirects) {

}

void SessionImplEV::SetCookies(const Cookies &cookies) {

}

void SessionImplEV::SetBody(body &&body) {

}

void SessionImplEV::SetBody(const body &body) {

}

const Url& SessionImplEV::url() const {
    return url_;
}
} //namespace HttpCli