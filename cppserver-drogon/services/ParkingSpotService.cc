/**
 * @file ParkingSpotService.cc
 * @brief ParkingSpot服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "ParkingSpotService.h"
#include <sstream>
#include <cstring>

namespace community {
namespace services {

// ========== 静态成员初始化 ==========
std::shared_ptr<ParkingSpotService> ParkingSpotService::s_instance_ = nullptr;
std::mutex ParkingSpotService::s_mutex_;

void ParkingSpotService::setInstance(std::shared_ptr<ParkingSpotService> instance) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_instance_ = std::move(instance);
}

std::shared_ptr<ParkingSpotService> ParkingSpotService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_instance_;
}

// ========== 查询所有 ==========
std::vector<models::ParkingSpot> ParkingSpotService::getAllParkingSpots() {
    std::vector<models::ParkingSpot> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        auto result = conn->query("SELECT id, spot_number, zone, type, current_vehicle_id, is_occupied, updated_at FROM parking_spots ORDER BY id");
        
        if (!result.success) {
            LOG_ERROR << "查询ParkingSpot列表失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::ParkingSpot::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getAllParkingSpots 异常: " << e.what();
    }

    return list;
}

// ========== 根据 ID 查询 ==========
std::optional<models::ParkingSpot> ParkingSpotService::getParkingSpotById(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "SELECT id, spot_number, zone, type, current_vehicle_id, is_occupied, updated_at FROM parking_spots WHERE id = " + std::to_string(id);
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        return models::ParkingSpot::fromDbRow(result.rows[0]);
    } catch (const std::exception& e) {
        LOG_ERROR << "getParkingSpotById 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 创建 ==========
int ParkingSpotService::createParkingSpot(const models::ParkingSpot& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        auto safeSpotNumber = conn->escape(obj.spotNumber);
        auto safeZone = conn->escape(obj.zone);
        auto safeType = conn->escape(obj.type);
        auto safeUpdatedAt = conn->escape(obj.updatedAt);

        std::ostringstream oss;
        oss << "INSERT INTO parking_spots (spot_number, zone, type, current_vehicle_id, is_occupied, updated_at) VALUES ("
           << "'" << safeSpotNumber << "'"
           << ", '" << safeZone << "'"
           << ", '" << safeType << "'"
           << ", " << obj.currentVehicleId
           << ", " << obj.isOccupied
           << ", '" << safeUpdatedAt << "'"
            << ");";

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "创建ParkingSpot失败: " << result.error;
            return -1;
        }

        LOG_INFO << "创建ParkingSpot成功，ID: " << result.insertId;
        return result.insertId;
    } catch (const std::exception& e) {
        LOG_ERROR << "createParkingSpot 异常: " << e.what();
        return -1;
    }
}

// ========== 更新 ==========
bool ParkingSpotService::updateParkingSpot(const models::ParkingSpot& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        std::ostringstream oss;
        oss << "UPDATE parking_spots SET ";

        std::vector<std::string> sets;
        
        if (!obj.spotNumber.empty()) {
            sets.push_back("spot_number = '" + conn->escape(obj.spotNumber) + "'");
        }
        if (!obj.zone.empty()) {
            sets.push_back("zone = '" + conn->escape(obj.zone) + "'");
        }
        if (!obj.type.empty()) {
            sets.push_back("type = '" + conn->escape(obj.type) + "'");
        }
        sets.push_back("current_vehicle_id = " + std::to_string(obj.currentVehicleId));
        sets.push_back("is_occupied = " + std::to_string(obj.isOccupied));
        if (!obj.updatedAt.empty()) {
            sets.push_back("updated_at = '" + conn->escape(obj.updatedAt) + "'");
        }

        if (sets.empty()) {
            LOG_WARN << "updateParkingSpot: 没有需要更新的字段";
            return false;
        }

        for (size_t i = 0; i < sets.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sets[i];
        }
        oss << " WHERE id = " << obj.id;

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "更新ParkingSpot失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updateParkingSpot 异常: " << e.what();
        return false;
    }
}

// ========== 删除 ==========
bool ParkingSpotService::deleteParkingSpot(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "DELETE FROM parking_spots WHERE id = " + std::to_string(id);
        auto result = conn->execute(sql);
        
        if (!result.success) {
            LOG_ERROR << "删除ParkingSpot失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "deleteParkingSpot 异常: " << e.what();
        return false;
    }
}

// ========== 分页查询 ==========
std::vector<models::ParkingSpot> ParkingSpotService::getParkingSpotsByPage(int page, int pageSize, int& total,
                                                          const std::string& keyword,
                                                          const std::string& filter1,
                                                          const std::string& filter2) {
    std::vector<models::ParkingSpot> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 构建 WHERE 条件
        std::vector<std::string> whereParts;
        if (!keyword.empty()) {
            std::string safeKw = conn->escape(keyword);
            whereParts.push_back("zone LIKE '%" + safeKw + "%'");
        }
        if (!filter1.empty()) {
            std::string safe = conn->escape(filter1);
            whereParts.push_back("type = '" + safe + "'");
        }
        if (!filter2.empty()) {
            std::string safe = conn->escape(filter2);
            whereParts.push_back("is_occupied = '" + safe + "'");
        }
        
        std::string whereClause = "";
        if (!whereParts.empty()) {
            whereClause = " WHERE " + whereParts[0];
            for (size_t i = 1; i < whereParts.size(); ++i) {
                whereClause += " AND " + whereParts[i];
            }
        }

        // 查询总数
        std::string countSql = "SELECT COUNT(*) as cnt FROM parking_spots" + whereClause;
        auto countResult = conn->query(countSql);
        if (countResult.success && !countResult.rows.empty()) {
            auto it = countResult.rows[0].find("cnt");
            if (it != countResult.rows[0].end()) {
                total = std::stoi(it->second);
            }
        }

        // 分页查询
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT id, spot_number, zone, type, current_vehicle_id, is_occupied, updated_at FROM parking_spots" + whereClause
                          + " ORDER BY id LIMIT " 
                          + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
        
        auto result = conn->query(sql);
        if (!result.success) {
            LOG_ERROR << "分页查询ParkingSpot失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::ParkingSpot::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getParkingSpotsByPage 异常: " << e.what();
        total = 0;
    }

    return list;
}

// ========== 车辆入场 ==========
bool ParkingSpotService::park(int id, int vehicleId) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::ostringstream oss;
        oss << "UPDATE parking_spots SET current_vehicle_id = " << vehicleId
            << ", is_occupied = 1, updated_at = NOW() WHERE id = " << id;
        auto result = conn->execute(oss.str());
        if (!result.success) {
            LOG_ERROR << "车辆入场失败: " << result.error;
            return false;
        }
        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "park 异常: " << e.what();
        return false;
    }
}

// ========== 车辆离场 ==========
bool ParkingSpotService::leave(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::ostringstream oss;
        oss << "UPDATE parking_spots SET current_vehicle_id = 0, is_occupied = 0,"
            << " updated_at = NOW() WHERE id = " << id;
        auto result = conn->execute(oss.str());
        if (!result.success) {
            LOG_ERROR << "车辆离场失败: " << result.error;
            return false;
        }
        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "leave 异常: " << e.what();
        return false;
    }
}

}  // namespace services
}  // namespace community
