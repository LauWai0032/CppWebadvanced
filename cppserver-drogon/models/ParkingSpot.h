/**
 * @file ParkingSpot.h
 * @brief ParkingSpot模型
 *
 * 对应数据库 parking_spots 表的实体类。
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
 * @struct ParkingSpot
 * @brief ParkingSpot模型结构体
 *
 * 简单的 POCO（Plain Old C++ Object），
 * 提供 toJson/fromDbRow 等转换方法。
 */
struct ParkingSpot {
    int         id                   = 0;
    std::string spotNumber;
    std::string zone;
    std::string type                 = "fixed";
    int         currentVehicleId     = 0;
    int         isOccupied           = 0;
    std::string updatedAt;

    ParkingSpot() = default;

    /**
     * @brief 转换为 JSON 对象
     */
    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id;
        j["spot_number"] = spotNumber;
        j["zone"] = zone;
        j["type"] = type;
        j["current_vehicle_id"] = currentVehicleId;
        j["is_occupied"] = isOccupied;
        j["updated_at"] = updatedAt;
        return j;
    }

    /**
     * @brief 从 JSON 创建 ParkingSpot 对象
     */
    static ParkingSpot fromJson(const Json::Value& j) {
        ParkingSpot obj;
        obj.id = j.get("id", 0).asInt();
        obj.spotNumber = j.get("spot_number", "").asString();
        obj.zone = j.get("zone", "").asString();
        obj.type = j.get("type", "fixed").asString();
        obj.currentVehicleId = j.get("current_vehicle_id", 0).asInt();
        obj.isOccupied = j.get("is_occupied", 0).asInt();
        obj.updatedAt = j.get("updated_at", "").asString();
        return obj;
    }

    /**
     * @brief 从数据库行创建 ParkingSpot 对象
     */
    static ParkingSpot fromDbRow(const DbRow& row) {
        ParkingSpot obj;
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
        obj.spotNumber = getStr("spot_number");
        obj.zone = getStr("zone");
        obj.type = getStr("type");
        obj.currentVehicleId = getInt("current_vehicle_id", 0);
        obj.isOccupied = getInt("is_occupied", 0);
        obj.updatedAt = getStr("updated_at");
        return obj;
    }
};

}  // namespace models
}  // namespace community
