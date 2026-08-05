/**
 * @file VehicleService.cc
 * @brief Vehicle服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "VehicleService.h"
#include <sstream>
#include <cstring>

namespace community {
namespace services {

// ========== 静态成员初始化 ==========
std::shared_ptr<VehicleService> VehicleService::s_instance_ = nullptr;
std::mutex VehicleService::s_mutex_;

void VehicleService::setInstance(std::shared_ptr<VehicleService> instance) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_instance_ = std::move(instance);
}

std::shared_ptr<VehicleService> VehicleService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_instance_;
}

// ========== 查询所有 ==========
std::vector<models::Vehicle> VehicleService::getAllVehicles() {
    std::vector<models::Vehicle> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        auto result = conn->query("SELECT id, user_id, plate_number, brand, color, status, created_at FROM vehicles ORDER BY id");
        
        if (!result.success) {
            LOG_ERROR << "查询Vehicle列表失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::Vehicle::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getAllVehicles 异常: " << e.what();
    }

    return list;
}

// ========== 根据 ID 查询 ==========
std::optional<models::Vehicle> VehicleService::getVehicleById(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "SELECT id, user_id, plate_number, brand, color, status, created_at FROM vehicles WHERE id = " + std::to_string(id);
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        return models::Vehicle::fromDbRow(result.rows[0]);
    } catch (const std::exception& e) {
        LOG_ERROR << "getVehicleById 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 创建 ==========
int VehicleService::createVehicle(const models::Vehicle& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        auto safePlateNumber = conn->escape(obj.plateNumber);
        auto safeBrand = conn->escape(obj.brand);
        auto safeColor = conn->escape(obj.color);
        auto safeCreatedAt = conn->escape(obj.createdAt);

        std::ostringstream oss;
        oss << "INSERT INTO vehicles (user_id, plate_number, brand, color, status, created_at) VALUES ("
           << obj.userId
           << ", '" << safePlateNumber << "'"
           << ", '" << safeBrand << "'"
           << ", '" << safeColor << "'"
           << ", " << obj.status
           << ", '" << safeCreatedAt << "'"
            << ");";

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "创建Vehicle失败: " << result.error;
            return -1;
        }

        LOG_INFO << "创建Vehicle成功，ID: " << result.insertId;
        return result.insertId;
    } catch (const std::exception& e) {
        LOG_ERROR << "createVehicle 异常: " << e.what();
        return -1;
    }
}

// ========== 更新 ==========
bool VehicleService::updateVehicle(const models::Vehicle& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        std::ostringstream oss;
        oss << "UPDATE vehicles SET ";

        std::vector<std::string> sets;
        
        sets.push_back("user_id = " + std::to_string(obj.userId));
        if (!obj.plateNumber.empty()) {
            sets.push_back("plate_number = '" + conn->escape(obj.plateNumber) + "'");
        }
        if (!obj.brand.empty()) {
            sets.push_back("brand = '" + conn->escape(obj.brand) + "'");
        }
        if (!obj.color.empty()) {
            sets.push_back("color = '" + conn->escape(obj.color) + "'");
        }
        sets.push_back("status = " + std::to_string(obj.status));
        if (!obj.createdAt.empty()) {
            sets.push_back("created_at = '" + conn->escape(obj.createdAt) + "'");
        }

        if (sets.empty()) {
            LOG_WARN << "updateVehicle: 没有需要更新的字段";
            return false;
        }

        for (size_t i = 0; i < sets.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sets[i];
        }
        oss << " WHERE id = " << obj.id;

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "更新Vehicle失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updateVehicle 异常: " << e.what();
        return false;
    }
}

// ========== 删除 ==========
bool VehicleService::deleteVehicle(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "DELETE FROM vehicles WHERE id = " + std::to_string(id);
        auto result = conn->execute(sql);
        
        if (!result.success) {
            LOG_ERROR << "删除Vehicle失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "deleteVehicle 异常: " << e.what();
        return false;
    }
}

// ========== 分页查询 ==========
std::vector<models::Vehicle> VehicleService::getVehiclesByPage(int page, int pageSize, int& total,
                                                          const std::string& keyword,
                                                          const std::string& filter1,
                                                          const std::string& filter2) {
    std::vector<models::Vehicle> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 构建 WHERE 条件
        std::vector<std::string> whereParts;
        if (!keyword.empty()) {
            std::string safeKw = conn->escape(keyword);
            whereParts.push_back("plate_number LIKE '%" + safeKw + "%'");
        }
        if (!filter1.empty()) {
            std::string safe = conn->escape(filter1);
            whereParts.push_back("user_id = '" + safe + "'");
        }
        
        std::string whereClause = "";
        if (!whereParts.empty()) {
            whereClause = " WHERE " + whereParts[0];
            for (size_t i = 1; i < whereParts.size(); ++i) {
                whereClause += " AND " + whereParts[i];
            }
        }

        // 查询总数
        std::string countSql = "SELECT COUNT(*) as cnt FROM vehicles" + whereClause;
        auto countResult = conn->query(countSql);
        if (countResult.success && !countResult.rows.empty()) {
            auto it = countResult.rows[0].find("cnt");
            if (it != countResult.rows[0].end()) {
                total = std::stoi(it->second);
            }
        }

        // 分页查询
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT id, user_id, plate_number, brand, color, status, created_at FROM vehicles" + whereClause
                          + " ORDER BY id LIMIT " 
                          + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
        
        auto result = conn->query(sql);
        if (!result.success) {
            LOG_ERROR << "分页查询Vehicle失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::Vehicle::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getVehiclesByPage 异常: " << e.what();
        total = 0;
    }

    return list;
}

}  // namespace services
}  // namespace community
