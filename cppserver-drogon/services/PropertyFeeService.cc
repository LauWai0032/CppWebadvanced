/**
 * @file PropertyFeeService.cc
 * @brief PropertyFee服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "PropertyFeeService.h"
#include <sstream>
#include <cstring>

namespace community {
namespace services {

// ========== 静态成员初始化 ==========
std::shared_ptr<PropertyFeeService> PropertyFeeService::s_instance_ = nullptr;
std::mutex PropertyFeeService::s_mutex_;

void PropertyFeeService::setInstance(std::shared_ptr<PropertyFeeService> instance) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_instance_ = std::move(instance);
}

std::shared_ptr<PropertyFeeService> PropertyFeeService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_instance_;
}

// ========== 查询所有 ==========
std::vector<models::PropertyFee> PropertyFeeService::getAllPropertyFees() {
    std::vector<models::PropertyFee> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        auto result = conn->query("SELECT id, house_id, fee_type, amount, bill_month, status, payment_method, paid_at, created_at FROM property_fees ORDER BY id");
        
        if (!result.success) {
            LOG_ERROR << "查询PropertyFee列表失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::PropertyFee::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getAllPropertyFees 异常: " << e.what();
    }

    return list;
}

// ========== 根据 ID 查询 ==========
std::optional<models::PropertyFee> PropertyFeeService::getPropertyFeeById(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "SELECT id, house_id, fee_type, amount, bill_month, status, payment_method, paid_at, created_at FROM property_fees WHERE id = " + std::to_string(id);
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        return models::PropertyFee::fromDbRow(result.rows[0]);
    } catch (const std::exception& e) {
        LOG_ERROR << "getPropertyFeeById 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 创建 ==========
int PropertyFeeService::createPropertyFee(const models::PropertyFee& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        auto safeFeeType = conn->escape(obj.feeType);
        auto safeBillMonth = conn->escape(obj.billMonth);
        auto safeStatus = conn->escape(obj.status);
        auto safePaymentMethod = conn->escape(obj.paymentMethod);
        auto safePaidAt = conn->escape(obj.paidAt);
        auto safeCreatedAt = conn->escape(obj.createdAt);

        std::ostringstream oss;
        oss << "INSERT INTO property_fees (house_id, fee_type, amount, bill_month, status, payment_method, paid_at, created_at) VALUES ("
           << obj.houseId
           << ", '" << safeFeeType << "'"
           << ", " << obj.amount
           << ", '" << safeBillMonth << "'"
           << ", '" << safeStatus << "'"
           << ", '" << safePaymentMethod << "'"
           << ", '" << safePaidAt << "'"
           << ", '" << safeCreatedAt << "'"
            << ");";

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "创建PropertyFee失败: " << result.error;
            return -1;
        }

        LOG_INFO << "创建PropertyFee成功，ID: " << result.insertId;
        return result.insertId;
    } catch (const std::exception& e) {
        LOG_ERROR << "createPropertyFee 异常: " << e.what();
        return -1;
    }
}

// ========== 更新 ==========
bool PropertyFeeService::updatePropertyFee(const models::PropertyFee& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        std::ostringstream oss;
        oss << "UPDATE property_fees SET ";

        std::vector<std::string> sets;
        
        sets.push_back("house_id = " + std::to_string(obj.houseId));
        if (!obj.feeType.empty()) {
            sets.push_back("fee_type = '" + conn->escape(obj.feeType) + "'");
        }
        sets.push_back("amount = " + std::to_string(obj.amount));
        if (!obj.billMonth.empty()) {
            sets.push_back("bill_month = '" + conn->escape(obj.billMonth) + "'");
        }
        if (!obj.status.empty()) {
            sets.push_back("status = '" + conn->escape(obj.status) + "'");
        }
        if (!obj.paymentMethod.empty()) {
            sets.push_back("payment_method = '" + conn->escape(obj.paymentMethod) + "'");
        }
        if (!obj.paidAt.empty()) {
            sets.push_back("paid_at = '" + conn->escape(obj.paidAt) + "'");
        }
        if (!obj.createdAt.empty()) {
            sets.push_back("created_at = '" + conn->escape(obj.createdAt) + "'");
        }

        if (sets.empty()) {
            LOG_WARN << "updatePropertyFee: 没有需要更新的字段";
            return false;
        }

        for (size_t i = 0; i < sets.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sets[i];
        }
        oss << " WHERE id = " << obj.id;

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "更新PropertyFee失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updatePropertyFee 异常: " << e.what();
        return false;
    }
}

// ========== 删除 ==========
bool PropertyFeeService::deletePropertyFee(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "DELETE FROM property_fees WHERE id = " + std::to_string(id);
        auto result = conn->execute(sql);
        
        if (!result.success) {
            LOG_ERROR << "删除PropertyFee失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "deletePropertyFee 异常: " << e.what();
        return false;
    }
}

// ========== 分页查询 ==========
std::vector<models::PropertyFee> PropertyFeeService::getPropertyFeesByPage(int page, int pageSize, int& total,
                                                          const std::string& keyword,
                                                          const std::string& filter1,
                                                          const std::string& filter2,
                                                          const std::string& filter3) {
    std::vector<models::PropertyFee> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 构建 WHERE 条件
        std::vector<std::string> whereParts;
        if (!keyword.empty()) {
            std::string safeKw = conn->escape(keyword);
            whereParts.push_back("fee_type LIKE '%" + safeKw + "%'");
        }
        if (!filter1.empty()) {
            std::string safe = conn->escape(filter1);
            whereParts.push_back("status = '" + safe + "'");
        }
        if (!filter2.empty()) {
            std::string safe = conn->escape(filter2);
            whereParts.push_back("bill_month = '" + safe + "'");
        }
        if (!filter3.empty()) {
            std::string safe = conn->escape(filter3);
            whereParts.push_back("house_id = '" + safe + "'");
        }
        
        std::string whereClause = "";
        if (!whereParts.empty()) {
            whereClause = " WHERE " + whereParts[0];
            for (size_t i = 1; i < whereParts.size(); ++i) {
                whereClause += " AND " + whereParts[i];
            }
        }

        // 查询总数
        std::string countSql = "SELECT COUNT(*) as cnt FROM property_fees" + whereClause;
        auto countResult = conn->query(countSql);
        if (countResult.success && !countResult.rows.empty()) {
            auto it = countResult.rows[0].find("cnt");
            if (it != countResult.rows[0].end()) {
                total = std::stoi(it->second);
            }
        }

        // 分页查询
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT id, house_id, fee_type, amount, bill_month, status, payment_method, paid_at, created_at FROM property_fees" + whereClause
                          + " ORDER BY id LIMIT " 
                          + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
        
        auto result = conn->query(sql);
        if (!result.success) {
            LOG_ERROR << "分页查询PropertyFee失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::PropertyFee::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getPropertyFeesByPage 异常: " << e.what();
        total = 0;
    }

    return list;
}

// ========== 缴费 ==========
bool PropertyFeeService::pay(int id, const std::string& paymentMethod) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string safeMethod = conn->escape(paymentMethod);
        std::ostringstream oss;
        oss << "UPDATE property_fees SET status = 'paid', payment_method = '"
            << safeMethod << "', paid_at = NOW() WHERE id = " << id;
        auto result = conn->execute(oss.str());
        if (!result.success) {
            LOG_ERROR << "缴费失败: " << result.error;
            return false;
        }
        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "pay 异常: " << e.what();
        return false;
    }
}

}  // namespace services
}  // namespace community
