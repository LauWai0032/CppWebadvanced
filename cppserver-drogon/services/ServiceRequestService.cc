/**
 * @file ServiceRequestService.cc
 * @brief ServiceRequest服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "ServiceRequestService.h"
#include <sstream>
#include <cstring>

namespace community {
namespace services {

// ========== 静态成员初始化 ==========
std::shared_ptr<ServiceRequestService> ServiceRequestService::s_instance_ = nullptr;
std::mutex ServiceRequestService::s_mutex_;

void ServiceRequestService::setInstance(std::shared_ptr<ServiceRequestService> instance) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_instance_ = std::move(instance);
}

std::shared_ptr<ServiceRequestService> ServiceRequestService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_instance_;
}

// ========== 查询所有 ==========
std::vector<models::ServiceRequest> ServiceRequestService::getAllServiceRequests() {
    std::vector<models::ServiceRequest> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        auto result = conn->query("SELECT id, user_id, type, title, description, status, created_at FROM service_requests ORDER BY id");
        
        if (!result.success) {
            LOG_ERROR << "查询ServiceRequest列表失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::ServiceRequest::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getAllServiceRequests 异常: " << e.what();
    }

    return list;
}

// ========== 根据 ID 查询 ==========
std::optional<models::ServiceRequest> ServiceRequestService::getServiceRequestById(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "SELECT id, user_id, type, title, description, status, created_at FROM service_requests WHERE id = " + std::to_string(id);
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        return models::ServiceRequest::fromDbRow(result.rows[0]);
    } catch (const std::exception& e) {
        LOG_ERROR << "getServiceRequestById 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 创建 ==========
int ServiceRequestService::createServiceRequest(const models::ServiceRequest& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        auto safeType = conn->escape(obj.type);
        auto safeTitle = conn->escape(obj.title);
        auto safeDescription = conn->escape(obj.description);
        auto safeStatus = conn->escape(obj.status);
        auto safeCreatedAt = conn->escape(obj.createdAt);

        std::ostringstream oss;
        oss << "INSERT INTO service_requests (user_id, type, title, description, status, created_at) VALUES ("
           << obj.userId
           << ", '" << safeType << "'"
           << ", '" << safeTitle << "'"
           << ", '" << safeDescription << "'"
           << ", '" << safeStatus << "'"
           << ", '" << safeCreatedAt << "'"
            << ");";

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "创建ServiceRequest失败: " << result.error;
            return -1;
        }

        LOG_INFO << "创建ServiceRequest成功，ID: " << result.insertId;
        return result.insertId;
    } catch (const std::exception& e) {
        LOG_ERROR << "createServiceRequest 异常: " << e.what();
        return -1;
    }
}

// ========== 更新 ==========
bool ServiceRequestService::updateServiceRequest(const models::ServiceRequest& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        std::ostringstream oss;
        oss << "UPDATE service_requests SET ";

        std::vector<std::string> sets;
        
        sets.push_back("user_id = " + std::to_string(obj.userId));
        if (!obj.type.empty()) {
            sets.push_back("type = '" + conn->escape(obj.type) + "'");
        }
        if (!obj.title.empty()) {
            sets.push_back("title = '" + conn->escape(obj.title) + "'");
        }
        if (!obj.description.empty()) {
            sets.push_back("description = '" + conn->escape(obj.description) + "'");
        }
        if (!obj.status.empty()) {
            sets.push_back("status = '" + conn->escape(obj.status) + "'");
        }
        if (!obj.createdAt.empty()) {
            sets.push_back("created_at = '" + conn->escape(obj.createdAt) + "'");
        }

        if (sets.empty()) {
            LOG_WARN << "updateServiceRequest: 没有需要更新的字段";
            return false;
        }

        for (size_t i = 0; i < sets.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sets[i];
        }
        oss << " WHERE id = " << obj.id;

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "更新ServiceRequest失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updateServiceRequest 异常: " << e.what();
        return false;
    }
}

// ========== 删除 ==========
bool ServiceRequestService::deleteServiceRequest(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "DELETE FROM service_requests WHERE id = " + std::to_string(id);
        auto result = conn->execute(sql);
        
        if (!result.success) {
            LOG_ERROR << "删除ServiceRequest失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "deleteServiceRequest 异常: " << e.what();
        return false;
    }
}

// ========== 分页查询 ==========
std::vector<models::ServiceRequest> ServiceRequestService::getServiceRequestsByPage(int page, int pageSize, int& total,
                                                          const std::string& keyword,
                                                          const std::string& filter1,
                                                          const std::string& filter2) {
    std::vector<models::ServiceRequest> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 构建 WHERE 条件
        std::vector<std::string> whereParts;
        if (!keyword.empty()) {
            std::string safeKw = conn->escape(keyword);
            whereParts.push_back("type LIKE '%" + safeKw + "%'");
        }
        if (!filter1.empty()) {
            std::string safe = conn->escape(filter1);
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
        std::string countSql = "SELECT COUNT(*) as cnt FROM service_requests" + whereClause;
        auto countResult = conn->query(countSql);
        if (countResult.success && !countResult.rows.empty()) {
            auto it = countResult.rows[0].find("cnt");
            if (it != countResult.rows[0].end()) {
                total = std::stoi(it->second);
            }
        }

        // 分页查询
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT id, user_id, type, title, description, status, created_at FROM service_requests" + whereClause
                          + " ORDER BY id LIMIT " 
                          + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
        
        auto result = conn->query(sql);
        if (!result.success) {
            LOG_ERROR << "分页查询ServiceRequest失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::ServiceRequest::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getServiceRequestsByPage 异常: " << e.what();
        total = 0;
    }

    return list;
}

// ========== 修改状态 ==========
bool ServiceRequestService::updateStatus(int id, const std::string& status) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string safeStatus = conn->escape(status);
        std::ostringstream oss;
        oss << "UPDATE service_requests SET status = '" << safeStatus
            << "' WHERE id = " << id;
        auto result = conn->execute(oss.str());
        if (!result.success) {
            LOG_ERROR << "修改状态失败: " << result.error;
            return false;
        }
        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updateStatus 异常: " << e.what();
        return false;
    }
}

}  // namespace services
}  // namespace community
