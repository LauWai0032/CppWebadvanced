/**
 * @file House.h
 * @brief House模型
 *
 * 对应数据库 houses 表的实体类。
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
 * @struct House
 * @brief House模型结构体
 *
 * 简单的 POCO（Plain Old C++ Object），
 * 提供 toJson/fromDbRow 等转换方法。
 */
struct House {
    int         id                   = 0;
    std::string buildingNo;
    std::string unitNo;
    std::string roomNo;
    double      area                 = 0.0;
    int         ownerId              = 0;

    House() = default;

    /**
     * @brief 转换为 JSON 对象
     */
    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id;
        j["building_no"] = buildingNo;
        j["unit_no"] = unitNo;
        j["room_no"] = roomNo;
        j["area"] = area;
        j["owner_id"] = ownerId;
        return j;
    }

    /**
     * @brief 从 JSON 创建 House 对象
     */
    static House fromJson(const Json::Value& j) {
        House obj;
        obj.id = j.get("id", 0).asInt();
        obj.buildingNo = j.get("building_no", "").asString();
        obj.unitNo = j.get("unit_no", "").asString();
        obj.roomNo = j.get("room_no", "").asString();
        obj.area = j.get("area", 0.0).asDouble();
        obj.ownerId = j.get("owner_id", 0).asInt();
        return obj;
    }

    /**
     * @brief 从数据库行创建 House 对象
     */
    static House fromDbRow(const DbRow& row) {
        House obj;
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
        obj.buildingNo = getStr("building_no");
        obj.unitNo = getStr("unit_no");
        obj.roomNo = getStr("room_no");
        obj.area = getDouble("area", 0.0);
        obj.ownerId = getInt("owner_id", 0);
        return obj;
    }
};

}  // namespace models
}  // namespace community
