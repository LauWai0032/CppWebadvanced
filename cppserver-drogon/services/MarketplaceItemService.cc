/**
 * @file MarketplaceItemService.cc
 * @brief MarketplaceItem服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "MarketplaceItemService.h"
#include <sstream>
#include <cstring>

namespace community {
namespace services {

// ========== 静态成员初始化 ==========
std::shared_ptr<MarketplaceItemService> MarketplaceItemService::s_instance_ = nullptr;
std::mutex MarketplaceItemService::s_mutex_;

void MarketplaceItemService::setInstance(std::shared_ptr<MarketplaceItemService> instance) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_instance_ = std::move(instance);
}

std::shared_ptr<MarketplaceItemService> MarketplaceItemService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_instance_;
}

// ========== 查询所有 ==========
std::vector<models::MarketplaceItem> MarketplaceItemService::getAllMarketplaceItems() {
    std::vector<models::MarketplaceItem> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        auto result = conn->query("SELECT id, seller_id, title, description, exchange_type, price, expect_item, status, created_at FROM marketplace_items ORDER BY id");
        
        if (!result.success) {
            LOG_ERROR << "查询MarketplaceItem列表失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::MarketplaceItem::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getAllMarketplaceItems 异常: " << e.what();
    }

    return list;
}

// ========== 根据 ID 查询 ==========
std::optional<models::MarketplaceItem> MarketplaceItemService::getMarketplaceItemById(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "SELECT id, seller_id, title, description, exchange_type, price, expect_item, status, created_at FROM marketplace_items WHERE id = " + std::to_string(id);
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        return models::MarketplaceItem::fromDbRow(result.rows[0]);
    } catch (const std::exception& e) {
        LOG_ERROR << "getMarketplaceItemById 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 创建 ==========
int MarketplaceItemService::createMarketplaceItem(const models::MarketplaceItem& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        auto safeTitle = conn->escape(obj.title);
        auto safeDescription = conn->escape(obj.description);
        auto safeExchangeType = conn->escape(obj.exchangeType);
        auto safeExpectItem = conn->escape(obj.expectItem);
        auto safeStatus = conn->escape(obj.status);
        auto safeCreatedAt = conn->escape(obj.createdAt);

        std::ostringstream oss;
        oss << "INSERT INTO marketplace_items (seller_id, title, description, exchange_type, price, expect_item, status, created_at) VALUES ("
           << obj.sellerId
           << ", '" << safeTitle << "'"
           << ", '" << safeDescription << "'"
           << ", '" << safeExchangeType << "'"
           << ", " << obj.price
           << ", '" << safeExpectItem << "'"
           << ", '" << safeStatus << "'"
           << ", '" << safeCreatedAt << "'"
            << ");";

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "创建MarketplaceItem失败: " << result.error;
            return -1;
        }

        LOG_INFO << "创建MarketplaceItem成功，ID: " << result.insertId;
        return result.insertId;
    } catch (const std::exception& e) {
        LOG_ERROR << "createMarketplaceItem 异常: " << e.what();
        return -1;
    }
}

// ========== 更新 ==========
bool MarketplaceItemService::updateMarketplaceItem(const models::MarketplaceItem& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        std::ostringstream oss;
        oss << "UPDATE marketplace_items SET ";

        std::vector<std::string> sets;
        
        sets.push_back("seller_id = " + std::to_string(obj.sellerId));
        if (!obj.title.empty()) {
            sets.push_back("title = '" + conn->escape(obj.title) + "'");
        }
        if (!obj.description.empty()) {
            sets.push_back("description = '" + conn->escape(obj.description) + "'");
        }
        if (!obj.exchangeType.empty()) {
            sets.push_back("exchange_type = '" + conn->escape(obj.exchangeType) + "'");
        }
        sets.push_back("price = " + std::to_string(obj.price));
        if (!obj.expectItem.empty()) {
            sets.push_back("expect_item = '" + conn->escape(obj.expectItem) + "'");
        }
        if (!obj.status.empty()) {
            sets.push_back("status = '" + conn->escape(obj.status) + "'");
        }
        if (!obj.createdAt.empty()) {
            sets.push_back("created_at = '" + conn->escape(obj.createdAt) + "'");
        }

        if (sets.empty()) {
            LOG_WARN << "updateMarketplaceItem: 没有需要更新的字段";
            return false;
        }

        for (size_t i = 0; i < sets.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sets[i];
        }
        oss << " WHERE id = " << obj.id;

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "更新MarketplaceItem失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updateMarketplaceItem 异常: " << e.what();
        return false;
    }
}

// ========== 删除 ==========
bool MarketplaceItemService::deleteMarketplaceItem(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "DELETE FROM marketplace_items WHERE id = " + std::to_string(id);
        auto result = conn->execute(sql);
        
        if (!result.success) {
            LOG_ERROR << "删除MarketplaceItem失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "deleteMarketplaceItem 异常: " << e.what();
        return false;
    }
}

// ========== 分页查询 ==========
std::vector<models::MarketplaceItem> MarketplaceItemService::getMarketplaceItemsByPage(int page, int pageSize, int& total,
                                                          const std::string& keyword,
                                                          const std::string& filter1,
                                                          const std::string& filter2) {
    std::vector<models::MarketplaceItem> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 构建 WHERE 条件
        std::vector<std::string> whereParts;
        if (!keyword.empty()) {
            std::string safeKw = conn->escape(keyword);
            whereParts.push_back("title LIKE '%" + safeKw + "%'");
        }
        if (!filter1.empty()) {
            std::string safe = conn->escape(filter1);
            whereParts.push_back("exchange_type = '" + safe + "'");
        }
        if (!filter2.empty()) {
            std::string safe = conn->escape(filter2);
            whereParts.push_back("status = '" + safe + "'");
        }
        
        std::string whereClause = "";
        if (!whereParts.empty()) {
            whereClause = " WHERE " + whereParts[0];
            for (size_t i = 1; i < whereParts.size(); ++i) {
                whereClause += " AND " + whereParts[i];
            }
        }

        // 查询总数
        std::string countSql = "SELECT COUNT(*) as cnt FROM marketplace_items" + whereClause;
        auto countResult = conn->query(countSql);
        if (countResult.success && !countResult.rows.empty()) {
            auto it = countResult.rows[0].find("cnt");
            if (it != countResult.rows[0].end()) {
                total = std::stoi(it->second);
            }
        }

        // 分页查询
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT id, seller_id, title, description, exchange_type, price, expect_item, status, created_at FROM marketplace_items" + whereClause
                          + " ORDER BY id LIMIT " 
                          + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
        
        auto result = conn->query(sql);
        if (!result.success) {
            LOG_ERROR << "分页查询MarketplaceItem失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::MarketplaceItem::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getMarketplaceItemsByPage 异常: " << e.what();
        total = 0;
    }

    return list;
}

}  // namespace services
}  // namespace community
