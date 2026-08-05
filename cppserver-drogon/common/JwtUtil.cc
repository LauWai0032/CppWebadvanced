/**
 * @file JwtUtil.cc
 * @brief JWT 工具类实现（HMAC-SHA256 + OpenSSL）
 * @date 2025
 */

#include "JwtUtil.h"
#include "Logger.h"

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <json/json.h>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cmath>

namespace community {

// ========== 静态成员初始化 ==========
std::string JwtUtil::s_secret_ = "community_system_default_secret_key_2025_please_change";
int         JwtUtil::s_defaultExpire_ = 86400;  // 24 小时

// ========== 设置密钥 ==========
void JwtUtil::setSecret(std::string_view secret) {
    s_secret_ = std::string(secret);
}

// ========== 设置默认过期时间 ==========
void JwtUtil::setDefaultExpireSeconds(int seconds) {
    s_defaultExpire_ = seconds;
}

// ========== Base64Url 编码 ==========
std::string JwtUtil::base64UrlEncode(const std::string& input) {
    static const char* encodeTable =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    std::string output;
    output.reserve((input.size() + 2) / 3 * 4);

    size_t i = 0;
    for (; i + 3 <= input.size(); i += 3) {
        uint32_t triple = (static_cast<unsigned char>(input[i]) << 16) |
                          (static_cast<unsigned char>(input[i + 1]) << 8) |
                          static_cast<unsigned char>(input[i + 2]);
        output.push_back(encodeTable[(triple >> 18) & 0x3F]);
        output.push_back(encodeTable[(triple >> 12) & 0x3F]);
        output.push_back(encodeTable[(triple >> 6) & 0x3F]);
        output.push_back(encodeTable[triple & 0x3F]);
    }

    // 处理剩余字节
    size_t remaining = input.size() - i;
    if (remaining == 1) {
        uint32_t triple = static_cast<unsigned char>(input[i]) << 16;
        output.push_back(encodeTable[(triple >> 18) & 0x3F]);
        output.push_back(encodeTable[(triple >> 12) & 0x3F]);
    } else if (remaining == 2) {
        uint32_t triple = (static_cast<unsigned char>(input[i]) << 16) |
                          (static_cast<unsigned char>(input[i + 1]) << 8);
        output.push_back(encodeTable[(triple >> 18) & 0x3F]);
        output.push_back(encodeTable[(triple >> 12) & 0x3F]);
        output.push_back(encodeTable[(triple >> 6) & 0x3F]);
    }

    return output;
}

// ========== Base64Url 解码 ==========
std::string JwtUtil::base64UrlDecode(const std::string& input) {
    static const int decodeTable[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,62,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,63,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };

    // 填充位数（JWT 使用无填充 Base64Url，需要补全）
    std::string padded = input;
    while (padded.size() % 4 != 0) {
        padded.push_back('=');
    }

    std::string output;
    output.reserve(padded.size() / 4 * 3);

    uint32_t buffer = 0;
    int bits = 0;

    for (char c : padded) {
        if (c == '=') break;
        int val = decodeTable[static_cast<unsigned char>(c)];
        if (val == -1) continue;  // 跳过非法字符
        buffer = (buffer << 6) | val;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            output.push_back(static_cast<char>((buffer >> bits) & 0xFF));
        }
    }

    return output;
}

// ========== HMAC-SHA256 签名 ==========
std::string JwtUtil::hmacSha256(std::string_view key, std::string_view data) {
    unsigned char result[SHA256_DIGEST_LENGTH];
    unsigned int resultLen = 0;

    HMAC(EVP_sha256(),
         key.data(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(),
         result, &resultLen);

    return std::string(reinterpret_cast<char*>(result), resultLen);
}

// ========== 生成 Token ==========
std::string JwtUtil::generateToken(int userId,
                                    std::string_view username,
                                    std::string_view role) {
    // 1. 构建 Header
    Json::Value header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";  // 紧凑格式
    std::string headerJson = Json::writeString(writer, header);
    std::string headerB64 = base64UrlEncode(headerJson);

    // 2. 构建 Payload
    auto now = std::chrono::system_clock::now();
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    auto expTimeT = nowTimeT + s_defaultExpire_;

    Json::Value payload;
    payload["sub"] = userId;
    payload["username"] = std::string(username);
    payload["role"] = std::string(role);
    payload["iat"] = static_cast<Json::Int64>(nowTimeT);
    payload["exp"] = static_cast<Json::Int64>(expTimeT);
    std::string payloadJson = Json::writeString(writer, payload);
    std::string payloadB64 = base64UrlEncode(payloadJson);

    // 3. 构建签名
    std::string signingInput = headerB64 + "." + payloadB64;
    std::string signature = hmacSha256(s_secret_, signingInput);
    std::string signatureB64 = base64UrlEncode(signature);

    // 4. 组装 Token
    return signingInput + "." + signatureB64;
}

// ========== 验证 Token ==========
std::optional<JwtPayload> JwtUtil::verifyToken(std::string_view token) {
    try {
        std::string tokenStr(token);

        // 1. 分割三段
        size_t firstDot = tokenStr.find('.');
        if (firstDot == std::string::npos) return std::nullopt;
        size_t secondDot = tokenStr.find('.', firstDot + 1);
        if (secondDot == std::string::npos) return std::nullopt;

        std::string headerB64    = tokenStr.substr(0, firstDot);
        std::string payloadB64   = tokenStr.substr(firstDot + 1, secondDot - firstDot - 1);
        std::string signatureB64 = tokenStr.substr(secondDot + 1);

        // 2. 验证签名
        std::string signingInput = headerB64 + "." + payloadB64;
        std::string expectedSig  = hmacSha256(s_secret_, signingInput);
        std::string expectedSigB64 = base64UrlEncode(expectedSig);

        if (signatureB64 != expectedSigB64) {
            LOG_WARN << "JWT 签名验证失败";
            return std::nullopt;
        }

        // 3. 解析 Payload
        std::string payloadJson = base64UrlDecode(payloadB64);
        Json::CharReaderBuilder reader;
        Json::Value payload;
        std::string errors;
        std::istringstream iss(payloadJson);
        if (!Json::parseFromStream(reader, iss, &payload, &errors)) {
            LOG_ERROR << "JWT Payload JSON 解析失败: " << errors;
            return std::nullopt;
        }

        // 4. 解析字段
        JwtPayload jwtPayload;
        jwtPayload.userId   = payload.get("sub", 0).asInt();
        jwtPayload.username = payload.get("username", "").asString();
        jwtPayload.role     = payload.get("role", "").asString();

        auto iat = payload.get("iat", 0).asInt64();
        auto exp = payload.get("exp", 0).asInt64();

        jwtPayload.issuedAt  = std::chrono::system_clock::from_time_t(static_cast<time_t>(iat));
        jwtPayload.expiresAt = std::chrono::system_clock::from_time_t(static_cast<time_t>(exp));

        // 5. 检查过期
        if (jwtPayload.isExpired()) {
            LOG_WARN << "JWT Token 已过期，用户: " << jwtPayload.username;
            return std::nullopt;
        }

        return jwtPayload;
    } catch (const std::exception& e) {
        LOG_ERROR << "JWT 验证异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 从 Authorization 头提取 Bearer Token ==========
std::optional<std::string> JwtUtil::extractBearerToken(std::string_view authHeader) {
    if (authHeader.size() <= 7) return std::nullopt;
    if (authHeader.substr(0, 7) != "Bearer ") return std::nullopt;
    return std::string(authHeader.substr(7));
}

}  // namespace community
