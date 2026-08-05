/**
 * @file PasswordUtil.h
 * @brief 密码加密工具类
 *
 * 使用 OpenSSL 的 SHA256 + 随机盐值实现密码哈希存储。
 * 存储格式：`salt$hash`（salt 为 16 字节随机值的十六进制表示，hash 为 SHA256 的十六进制表示）。
 *
 * 说明：
 *   - 由于环境中可能没有 bcrypt 库，采用 SHA256 + salt 的方式。
 *   - 虽然不如 bcrypt/argon2 抗暴力破解，但远优于明文存储和简单哈希。
 *   - 如需更高安全性，可替换为 bcrypt（需安装 libbcrypt-dev）。
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>

namespace community {

/**
 * @class PasswordUtil
 * @brief 密码哈希工具类
 *
 * 提供密码哈希生成和验证功能，基于 OpenSSL 的 SHA256 实现。
 */
class PasswordUtil {
public:
    /**
     * @brief 对明文密码进行哈希
     * @param password 明文密码
     * @return std::string 哈希后的密码（格式：salt$hash）
     */
    static std::string hashPassword(std::string_view password);

    /**
     * @brief 验证密码
     * @param password     明文密码
     * @param storedHash   存储的哈希值（格式：salt$hash）
     * @return bool 是否匹配
     */
    static bool verifyPassword(std::string_view password, std::string_view storedHash);

private:
    PasswordUtil() = delete;  // 禁止实例化

    /**
     * @brief 生成随机盐值（十六进制字符串）
     * @param byteCount 盐值字节数（默认 16）
     * @return std::string 十六进制表示的盐值
     */
    static std::string generateSalt(int byteCount = 16);

    /**
     * @brief 计算 SHA256 哈希（返回十六进制字符串）
     * @param input 输入字符串
     * @return std::string 64 位十六进制哈希值
     */
    static std::string sha256Hex(std::string_view input);

    /**
     * @brief 字节数组转十六进制字符串
     */
    static std::string bytesToHex(const unsigned char* bytes, size_t len);
};

}  // namespace community
