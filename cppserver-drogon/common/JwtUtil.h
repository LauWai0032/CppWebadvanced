/**
 * @file JwtUtil.h
 * @brief JWT（JSON Web Token）工具类
 *
 * 使用 HMAC-SHA256 签名算法（基于 OpenSSL 实现）。
 * 用于生成和验证用户登录 Token，实现无状态认证。
 *
 * Token 结构：
 *   Header.Payload.Signature
 *   - Header:  类型 + 算法（{"alg":"HS256","typ":"JWT"}）
 *   - Payload: 载荷（用户ID、用户名、角色、过期时间等）
 *   - Signature: HMAC-SHA256(Header + "." + Payload, secret)
 *
 * 设计模式：
 *   - 静态工具类：无需实例化，直接调用静态方法
 *   - Base64Url 编码：符合 JWT 规范的 URL 安全 Base64 编码
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <chrono>

namespace community {

/**
 * @struct JwtPayload
 * @brief JWT 载荷数据结构
 *
 * 包含标准声明（sub、iat、exp）和业务声明（用户名、角色）。
 */
struct JwtPayload {
    int         userId      = 0;       ///< 用户 ID（对应 sub 声明）
    std::string username;              ///< 用户名
    std::string role;                  ///< 用户角色（admin/property/owner）
    std::chrono::system_clock::time_point issuedAt;   ///< 签发时间（iat）
    std::chrono::system_clock::time_point expiresAt;  ///< 过期时间（exp）

    /**
     * @brief 是否已过期
     */
    [[nodiscard]] bool isExpired() const {
        return std::chrono::system_clock::now() > expiresAt;
    }
};

/**
 * @class JwtUtil
 * @brief JWT 工具类（HMAC-SHA256）
 *
 * 提供 Token 生成和验证功能，基于 OpenSSL 的 HMAC 实现。
 * 所有方法均为静态方法，使用前需设置密钥。
 */
class JwtUtil {
public:
    /**
     * @brief 设置 JWT 签名密钥
     * @param secret 密钥字符串（建议至少 32 字节）
     */
    static void setSecret(std::string_view secret);

    /**
     * @brief 设置 Token 默认过期时间（秒）
     * @param seconds 过期秒数，默认 24 小时
     */
    static void setDefaultExpireSeconds(int seconds);

    /**
     * @brief 生成 JWT Token
     * @param userId   用户 ID
     * @param username 用户名
     * @param role     用户角色
     * @return std::string JWT Token 字符串
     */
    static std::string generateToken(int userId,
                                     std::string_view username,
                                     std::string_view role);

    /**
     * @brief 验证 JWT Token 并解析载荷
     * @param token JWT Token 字符串
     * @return std::optional<JwtPayload> 验证成功返回载荷，失败返回 nullopt
     */
    static std::optional<JwtPayload> verifyToken(std::string_view token);

    /**
     * @brief 从 Authorization 请求头中提取 Bearer Token
     * @param authHeader Authorization 头值（如 "Bearer xxx.yyy.zzz"）
     * @return std::optional<std::string> 提取成功返回 Token，失败返回 nullopt
     */
    static std::optional<std::string> extractBearerToken(std::string_view authHeader);

private:
    JwtUtil() = delete;  // 禁止实例化

    /**
     * @brief Base64Url 编码
     */
    static std::string base64UrlEncode(const std::string& input);

    /**
     * @brief Base64Url 解码
     */
    static std::string base64UrlDecode(const std::string& input);

    /**
     * @brief HMAC-SHA256 签名
     * @param key  密钥
     * @param data 待签名数据
     * @return std::string 签名结果（二进制）
     */
    static std::string hmacSha256(std::string_view key, std::string_view data);

    // ========== 静态成员 ==========
    static std::string s_secret_;          ///< 签名密钥
    static int         s_defaultExpire_;   ///< 默认过期时间（秒）
};

}  // namespace community
