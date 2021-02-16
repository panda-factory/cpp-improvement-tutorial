//
// Created by admin on 2021/2/4.
//

#ifndef TEST_WEB_SOCKET_CLI_SESSION_IMPL_EV_H
#define TEST_WEB_SOCKET_CLI_SESSION_IMPL_EV_H

#include <vector>

#include "net/ws_cli/session_impl.h"
#include "net/ws_cli/header.h"
#include "net/ws_cli/implement/ev/ws_state.h"
#include "net/ws_cli/ws_types.h"
#include "utf8_utils.h"

#include <event2/dns.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

namespace ws {
class SessionImplEV : public SessionImpl {
public:
    int DoConnect(const std::string& server, int port, const std::string& uri) override;
    int DoInit() override;
    int DoSendMsg(char *msg) override;

    void SetHandler(const ConnectHandler& onConnect) override;
    void SetHandler(const MessageHandler& onMessage) override;

    SessionImplEV() = default;
private:
    static void BuiltinNoCopyCleanupWrapper(const void *data, size_t dataLen, void *extra);
    static void OnCloseTimeout(evutil_socket_t fd, short what, void *arg);
    static void OnConnectionTimeout(evutil_socket_t fd, short what, void *arg);
    static void OnRead(bufferevent* bev, void* ObscureData);
    static void OnWrite(bufferevent* bev, void* ObscureData);
    static void OnEvent(bufferevent* bev, short events, void* ObscureData);
    int CheckServerProtocolList(const char *val);
    int CloseWithStatus(WSCloseStatus status);
    int CloseWithStatusReason(WSCloseStatus status, const char *reason, size_t reason_len);
    int GetRandomMask(char *buf, size_t len);
    int GenerateHandshakeKey();
    const std::string GetUri(char *buf, size_t bufsize);
    int HandleCloseFrame();
    int HandleControlFrame();
    int HandleFrameBegin();
    int HandleFrameData(char *buf, size_t len);
    int HandleFrameEnd();
    int HandlePingFrame();
    int HandlePongFrame();
    void OnConnectEvent(bufferevent* bev, short events);
    void OnEofEvent(struct bufferevent *bev, short events);
    int OnSendMsgBegin(int binary);
    int OnSendMsgEnd();
    void OnMsgBegin();
    void OnMsgEnd(void *arg);
    void OnMsgFrame(char *payload, uint64_t len, void *arg);
    void OnMsgFrameData(char *payload, uint64_t len, void *arg);
    void OnMsgFrameBegin(void *arg);
    void OnMsgFrameEnd(void *arg);
    int OnMsgFrameDataBegin(uint64_t datalen);
    int OnMsgFrameDataSend(char *data, uint64_t datalen);
    int OnMsgFrameSend(char *frame_data, uint64_t datalen);
    int ParseHttpHeader(char *line, std::string& headerKey, std::string& headerVal);
    int ParseHttpStatus(const char *line,
                        int& httpMajorVersion,
                        int& httpMinorVersion,
                        int& statusCode);
    WSParseState ReadHttpHeaders(struct evbuffer *in);
    WSParseState ReadHttpStatus(
            struct evbuffer *inBuf,
            int& httpMajorVersion,
            int& httpMinorVersion,
            int& statusCode);
    WSParseState ReadServerHandshakeReply(struct evbuffer *in);
    void Request(bufferevent* bev);
    void ReadWebsocket(struct evbuffer *in);
    int SendMsgEx(char *msg, uint64_t len, int binary);
    int SendClose(WSCloseStatus statusCode, const char *reason, size_t reason_len);
    int SendData(char *msg, uint64_t len, int no_copy);
    int SendFrameRaw(WSOpCode opcode, char *data, uint64_t dataLen);
    int SendHandshake(struct evbuffer *out);
    int SetupConnectionTimeout();
    int SetupTimeoutEvent(struct event **ev, struct timeval *tv);
    void ShutDown();
    int CalculateKeyHash(const char *handshake_key_base64,
                                        char *key_hash, size_t len);
    int ValidateHeader();
    int ValidateHttpHeader(uint8_t flag,
                           const std::string& name, const std::string& val,
                           const std::string& expected_name, const std::string& expected_val,
                           int must_appear_only_once);
    int ValidateHttpHeaders(const std::string& name, const std::string& val);
    WSParseState UnpackHeader(size_t& headerLen, const unsigned char *b, size_t len);

    struct event_base *evBase_ = nullptr;
    struct evdns_base *dnsBase_ = nullptr;
    struct evbuffer *msg_ = nullptr;
    struct evbuffer *frameData_ = nullptr;
    struct bufferevent *bev_ = nullptr;
    struct timeval connectTimeout_ = {0};
    struct event *connectTimeoutEvent_ = nullptr;
    std::string uri_;
    std::string host_;
    std::string handshakeKeyBase64_;
    std::string origin_;
    int port_;
    size_t numSubprotocols_;
    std::vector<std::string> subprotocols_;
    WSConnectState connectState_;
    struct event *closeTimeoutEvent_;
    uint8_t httpHeaderFlags_;
    uint64_t recvFrameLen_;
    uint64_t maxFrameSize_;    ///< The max frame size to allow before chunking.
    char ctrlPayload_[WS_CONTROL_MAX_PAYLOAD_LEN];
                ///< Control frame payload.
    size_t ctrlLen_;            ///< Length of the control payload.
    WSState state_;
    Utf8State utf8State_; ///< Current state of utf8 validator.
    WSSendState sendState_;
    WSCloseStatus serverCloseStatus_;
    bool hasHeader_;             ///< Has the websocket header been read yet?
    WSHeader wsHeader_;
    Header header_;
    bool inMsgFlag_;
    bool useSsl_;
    int binaryMode_;
    bool msgIsBinary_;           ///< The opcode of the current message.
    bool sentCloseFlag_;
    bool receivedCloseFlag_;         ///< Did we receive a close frame?
    bool msgIsBinary;
    NoCopyCleanup noCopyCleanupCb_;
    void *noCopyExtra_;        ///< User supplied argument for
                            /// the ws_s#no_copy_cleanup_cb
    CloseCallback onClose_;
    ConnectHandler onConnect_;
    MessageHandler onMessage_;
    MsgBeginCallback onMsgBegin_;
    MsgEndCallback onMsgEnd_;
    MsgFrameBeginCallback onMsgFrameBegin_;
    MsgFrameEndCallback onMsgFrameEnd_;
    MsgFrameDataCallback onMsgFrameData_;
    MsgCallback onPing_;
    MsgCallback onPong_;

    char* serverReason_;
    size_t serverReasonLen_;
};

} // namespace ws


#endif //TEST_WEB_SOCKET_CLI_SESSION_IMPL_EV_H
