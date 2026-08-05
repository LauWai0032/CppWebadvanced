/**
 * @file ServiceRequest.h
 * @brief ServiceRequest模型
 *
 * 对应数据库 service_requests 表的实体类。
 * 使用 C++20 特性，提供现代化的数据模型定义。
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <json/json.h>
#include "common/DbPool.h"

namespace community {
namespace models {

/**
 * @struct ServiceRequest
 * @brief ServiceRequest模型结构体
 *
 * 简单的 POCO（Plain Old C++ Object），
 * 提供 toJson/fromDbRow 等转换方法。
 */
struct ServiceRequest {
    int         id                   = 0;
    int         userId               = 0;
    std::string type                 = "repair";
    std::string title;
    std::string description;
    std::string status               = "pending";
    std::string createdAt;

    ServiceRequest() = default;

    /**
     * @brief 转换为 JSON 对象
     */
    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id;
        j["user_id"] = userId;
        j["type"] = type;
        j["title"] = title;
        j["description"] = description;
        j["status"] = status;
        j["created_at"] = createdAt;
        return j;
    }

    /**
     * @brief 从 JSON 创建 ServiceRequest 对象
     */
    static ServiceRequest fromJson(const Json::Value& j) {
        ServiceRequest obj;
        obj.id = j.get("id", 0).asInt();
        obj.userId = j.get("user_id", 0).asInt();
        obj.type = j.get("type", "repair").asString();
        obj.title = j.get("title", "").asString();
        obj.description = j.get("description", "").asString();
        obj.status = j.get("status", "pending").asString();
        obj.createdAt = j.get("created_at", "").asString();
        return obj;
    }

    /**
     * @brief 从数据库行创建 ServiceRequest 对象
     */
    static ServiceRequest fromDbRow(const DbRow& row) {
        ServiceRequest obj;
        auto getStr = [&](const std::string& key) -> std::string {
            auto it = row.find(key);
            return (it != row.end()) ? it->second : "";
        };
        auto getInt = [&](const std::string& key, int def = 0) -> int {
            auto s = getStr(key);
            if (s.empty()) return def;
            try { return std::stoi(s); } catch (...) { return def; }
        };
        auto getDouble = [&](const std::string& key, double def = 0.0) -> double {
            auto s = getStr(key);
            if (s.empty()) return def;
            try { return std::stod(s); } catch (...) { return def; }
        };

        obj.id = getInt("id", 0);
        obj.userId = getInt("user_id", 0);
        obj.type = getStr("type");
        obj.title = getStr("title");
        obj.description = getStr("description");
        obj.status = getStr("status");
        obj.createdAt = getStr("created_at");
        return obj;
    }
};

}  // namespace models
}  // namespace community
