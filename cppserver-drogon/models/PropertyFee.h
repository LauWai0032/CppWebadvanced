/**
 * @file PropertyFee.h
 * @brief PropertyFee模型
 *
 * 对应数据库 property_fees 表的实体类。
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
 * @struct PropertyFee
 * @brief PropertyFee模型结构体
 *
 * 简单的 POCO（Plain Old C++ Object），
 * 提供 toJson/fromDbRow 等转换方法。
 */
struct PropertyFee {
    int         id                   = 0;
    int         houseId              = 0;
    std::string feeType              = "property";
    double      amount               = 0.0;
    std::string billMonth;
    std::string status               = "unpaid";
    std::string paymentMethod;
    std::string paidAt;
    std::string createdAt;

    PropertyFee() = default;

    /**
     * @brief 转换为 JSON 对象
     */
    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id;
        j["house_id"] = houseId;
        j["fee_type"] = feeType;
        j["amount"] = amount;
        j["bill_month"] = billMonth;
        j["status"] = status;
        j["payment_method"] = paymentMethod;
        j["paid_at"] = paidAt;
        j["created_at"] = createdAt;
        return j;
    }

    /**
     * @brief 从 JSON 创建 PropertyFee 对象
     */
    static PropertyFee fromJson(const Json::Value& j) {
        PropertyFee obj;
        obj.id = j.get("id", 0).asInt();
        obj.houseId = j.get("house_id", 0).asInt();
        obj.feeType = j.get("fee_type", "property").asString();
        obj.amount = j.get("amount", 0.0).asDouble();
        obj.billMonth = j.get("bill_month", "").asString();
        obj.status = j.get("status", "unpaid").asString();
        obj.paymentMethod = j.get("payment_method", "").asString();
        obj.paidAt = j.get("paid_at", "").asString();
        obj.createdAt = j.get("created_at", "").asString();
        return obj;
    }

    /**
     * @brief 从数据库行创建 PropertyFee 对象
     */
    static PropertyFee fromDbRow(const DbRow& row) {
        PropertyFee obj;
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
        obj.houseId = getInt("house_id", 0);
        obj.feeType = getStr("fee_type");
        obj.amount = getDouble("amount", 0.0);
        obj.billMonth = getStr("bill_month");
        obj.status = getStr("status");
        obj.paymentMethod = getStr("payment_method");
        obj.paidAt = getStr("paid_at");
        obj.createdAt = getStr("created_at");
        return obj;
    }
};

}  // namespace models
}  // namespace community
