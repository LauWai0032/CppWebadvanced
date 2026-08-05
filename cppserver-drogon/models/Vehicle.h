/**
 * @file Vehicle.h
 * @brief Vehicle模型
 *
 * 对应数据库 vehicles 表的实体类。
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
 * @struct Vehicle
 * @brief Vehicle模型结构体
 *
 * 简单的 POCO（Plain Old C++ Object），
 * 提供 toJson/fromDbRow 等转换方法。
 */
struct Vehicle {
    int         id                   = 0;
    int         userId               = 0;
    std::string plateNumber;
    std::string brand;
    std::string color;
    int         status               = 1;
    std::string createdAt;

    Vehicle() = default;

    /**
     * @brief 转换为 JSON 对象
     */
    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id;
        j["user_id"] = userId;
        j["plate_number"] = plateNumber;
        j["brand"] = brand;
        j["color"] = color;
        j["status"] = status;
        j["created_at"] = createdAt;
        return j;
    }

    /**
     * @brief 从 JSON 创建 Vehicle 对象
     */
    static Vehicle fromJson(const Json::Value& j) {
        Vehicle obj;
        obj.id = j.get("id", 0).asInt();
        obj.userId = j.get("user_id", 0).asInt();
        obj.plateNumber = j.get("plate_number", "").asString();
        obj.brand = j.get("brand", "").asString();
        obj.color = j.get("color", "").asString();
        obj.status = j.get("status", 1).asInt();
        obj.createdAt = j.get("created_at", "").asString();
        return obj;
    }

    /**
     * @brief 从数据库行创建 Vehicle 对象
     */
    static Vehicle fromDbRow(const DbRow& row) {
        Vehicle obj;
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
        obj.plateNumber = getStr("plate_number");
        obj.brand = getStr("brand");
        obj.color = getStr("color");
        obj.status = getInt("status", 1);
        obj.createdAt = getStr("created_at");
        return obj;
    }
};

}  // namespace models
}  // namespace community
