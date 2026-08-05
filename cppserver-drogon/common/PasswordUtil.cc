/**
 * @file PasswordUtil.cc
 * @brief 密码加密工具实现（SHA256 + Salt + OpenSSL）
 * @date 2025
 */

#include "PasswordUtil.h"
#include "Logger.h"

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <vector>

namespace community {

// ========== 字节数组转十六进制字符串 ==========
std::string PasswordUtil::bytesToHex(const unsigned char* bytes, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

// ========== 生成随机盐值 ==========
std::string PasswordUtil::generateSalt(int byteCount) {
    std::vector<unsigned char> salt(byteCount);

    // 使用 OpenSSL 的 CSPRNG
    if (RAND_bytes(salt.data(), byteCount) != 1) {
        // Fallback：使用标准库随机数（安全性略低，但可用）
        LOG_WARN << "OpenSSL RAND_bytes 失败，使用备用随机数生成器";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned char> dist(0, 255);
        for (int i = 0; i < byteCount; ++i) {
            salt[i] = dist(gen);
        }
    }

    return bytesToHex(salt.data(), salt.size());
}

// ========== 计算 SHA256 哈希（返回十六进制） ==========
std::string PasswordUtil::sha256Hex(std::string_view input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.data()),
           input.size(), hash);
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

// ========== 哈希密码 ==========
std::string PasswordUtil::hashPassword(std::string_view password) {
    // 1. 生成随机盐值
    std::string salt = generateSalt();

    // 2. 计算：SHA256(salt + password)
    std::string combined = salt + std::string(password);
    std::string hash = sha256Hex(combined);

    // 3. 返回格式：salt$hash
    return salt + "$" + hash;
}

// ========== 验证密码 ==========
bool PasswordUtil::verifyPassword(std::string_view password, std::string_view storedHash) {
    // 存储格式：salt$hash
    size_t pos = storedHash.find('$');
    if (pos == std::string_view::npos) {
        // 旧格式或明文（兼容处理，返回 false）
        LOG_WARN << "密码哈希格式不正确（缺少 $ 分隔符）";
        return false;
    }

    std::string_view salt = storedHash.substr(0, pos);
    std::string_view expectedHash = storedHash.substr(pos + 1);

    // 使用相同的盐值和算法计算
    std::string combined = std::string(salt) + std::string(password);
    std::string actualHash = sha256Hex(combined);

    // 比较哈希值（使用 string 比较，简单场景足够）
    return actualHash == expectedHash;
}

}  // namespace community
