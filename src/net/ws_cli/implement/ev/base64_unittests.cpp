//
// Created by admin on 2021/2/17.
//


#include "gtest/gtest.h"
#include "base64.h"
TEST(Base64Test, Encode1) {
    std::string str{"abcd"};
    std::string base64Str = EncodeBase64(reinterpret_cast<const unsigned char*>(str.c_str()), str.size());
    ASSERT_EQ(base64Str, "YWJjZA==");
}
TEST(Base64Test, Decode1) {
    std::string baseStr{"YWJjZA=="};
    std::string decodeStr = DecodeBase64(baseStr);
    ASSERT_EQ(decodeStr, "abcd");
}

TEST(Base64Test, Encode2) {
    std::string str{"A"};
    std::string base64Str = EncodeBase64(reinterpret_cast<const unsigned char*>(str.c_str()), str.size());
    ASSERT_EQ(base64Str, "QQ==");
}
TEST(Base64Test, Decode2) {
    std::string baseStr{"QQ=="};
    std::string decodeStr = DecodeBase64(baseStr);
    ASSERT_EQ(decodeStr, "A");
}

TEST(Base64Test, Encode3) {
    std::string str{"BC"};
    std::string base64Str = EncodeBase64(reinterpret_cast<const unsigned char*>(str.c_str()), str.size());
    ASSERT_EQ(base64Str, "QkM=");
}
TEST(Base64Test, Decode3) {
    std::string baseStr{"QkM="};
    std::string decodeStr = DecodeBase64(baseStr);
    ASSERT_EQ(decodeStr, "BC");
}

TEST(Base64Test, Encode4) {
    std::string str{"Man"};
    std::string base64Str = EncodeBase64(reinterpret_cast<const unsigned char*>(str.c_str()), str.size());
    ASSERT_EQ(base64Str, "TWFu");
}
TEST(Base64Test, Decode4) {
    std::string baseStr{"TWFu"};
    std::string decodeStr = DecodeBase64(baseStr);
    ASSERT_EQ(decodeStr, "Man");
}

TEST(Base64Test, Encode5) {
    std::string str{"5qyi6L+O5L2/55So5pys5bel5YW3"};
    std::string base64Str = EncodeBase64(reinterpret_cast<const unsigned char*>(str.c_str()), str.size());
    ASSERT_EQ(base64Str, "NXF5aTZMK081TDIvNTVTbzVweXM1YmVsNVlXMw==");
}
TEST(Base64Test, Decode5) {
    std::string baseStr{"NXF5aTZMK081TDIvNTVTbzVweXM1YmVsNVlXMw=="};
    std::string decodeStr = DecodeBase64(baseStr);
    ASSERT_EQ(decodeStr, "5qyi6L+O5L2/55So5pys5bel5YW3");
}

