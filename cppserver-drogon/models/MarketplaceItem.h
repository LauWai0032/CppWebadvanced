/**
 * @file MarketplaceItem.h
 * @brief MarketplaceItem模型
 *
 * 对应数据库 marketplace_items 表的实体类。
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
 * @struct MarketplaceItem
 * @brief MarketplaceItem模型结构体
 *
 * 简单的 POCO（Plain Old C++ Object），
 * 提供 toJson/fromDbRow 等转换方法。
 */
struct MarketplaceItem {
    int         id                   = 0;
    int         sellerId             = 0;
    std::string title;
    std::string description;
    std::string exchangeType         = "sell";
    double      price                = 0.0;
    std::string expectItem;
    std::string status               = "active";
    std::string createdAt;

    MarketplaceItem() = default;

    /**
     * @brief 转换为 JSON 对象
     */
    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id;
        j["seller_id"] = sellerId;
        j["title"] = title;
        j["description"] = description;
        j["exchange_type"] = exchangeType;
        j["price"] = price;
        j["expect_item"] = expectItem;
        j["status"] = status;
        j["created_at"] = createdAt;
        return j;
    }

    /**
     * @brief 从 JSON 创建 MarketplaceItem 对象
     */
    static MarketplaceItem fromJson(const Json::Value& j) {
        MarketplaceItem obj;
        obj.id = j.get("id", 0).asInt();
        obj.sellerId = j.get("seller_id", 0).asInt();
        obj.title = j.get("title", "").asString();
        obj.description = j.get("description", "").asString();
        obj.exchangeType = j.get("exchange_type", "sell").asString();
        obj.price = j.get("price", 0.0).asDouble();
        obj.expectItem = j.get("expect_item", "").asString();
        obj.status = j.get("status", "active").asString();
        obj.createdAt = j.get("created_at", "").asString();
        return obj;
    }

    /**
     * @brief 从数据库行创建 MarketplaceItem 对象
     */
    static MarketplaceItem fromDbRow(const DbRow& row) {
        MarketplaceItem obj;
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
        obj.sellerId = getInt("seller_id", 0);
        obj.title = getStr("title");
        obj.description = getStr("description");
        obj.exchangeType = getStr("exchange_type");
        obj.price = getDouble("price", 0.0);
        obj.expectItem = getStr("expect_item");
        obj.status = getStr("status");
        obj.createdAt = getStr("created_at");
        return obj;
    }
};

}  // namespace models
}  // namespace community
