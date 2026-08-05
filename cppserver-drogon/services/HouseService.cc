/**
 * @file HouseService.cc
 * @brief House服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "HouseService.h"
#include <sstream>
#include <cstring>

namespace community {
namespace services {

// ========== 静态成员初始化 ==========
std::shared_ptr<HouseService> HouseService::s_instance_ = nullptr;
std::mutex HouseService::s_mutex_;

void HouseService::setInstance(std::shared_ptr<HouseService> instance) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_instance_ = std::move(instance);
}

std::shared_ptr<HouseService> HouseService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_instance_;
}

// ========== 查询所有 ==========
std::vector<models::House> HouseService::getAllHouses() {
    std::vector<models::House> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        auto result = conn->query("SELECT id, building_no, unit_no, room_no, area, owner_id FROM houses ORDER BY id");
        
        if (!result.success) {
            LOG_ERROR << "查询House列表失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::House::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getAllHouses 异常: " << e.what();
    }

    return list;
}

// ========== 根据 ID 查询 ==========
std::optional<models::House> HouseService::getHouseById(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "SELECT id, building_no, unit_no, room_no, area, owner_id FROM houses WHERE id = " + std::to_string(id);
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        return models::House::fromDbRow(result.rows[0]);
    } catch (const std::exception& e) {
        LOG_ERROR << "getHouseById 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 创建 ==========
int HouseService::createHouse(const models::House& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        auto safeBuildingNo = conn->escape(obj.buildingNo);
        auto safeUnitNo = conn->escape(obj.unitNo);
        auto safeRoomNo = conn->escape(obj.roomNo);

        std::ostringstream oss;
        oss << "INSERT INTO houses (building_no, unit_no, room_no, area, owner_id) VALUES ("
           << "'" << safeBuildingNo << "'"
           << ", '" << safeUnitNo << "'"
           << ", '" << safeRoomNo << "'"
           << ", " << obj.area
           << ", " << obj.ownerId
            << ");";

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "创建House失败: " << result.error;
            return -1;
        }

        LOG_INFO << "创建House成功，ID: " << result.insertId;
        return result.insertId;
    } catch (const std::exception& e) {
        LOG_ERROR << "createHouse 异常: " << e.what();
        return -1;
    }
}

// ========== 更新 ==========
bool HouseService::updateHouse(const models::House& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        std::ostringstream oss;
        oss << "UPDATE houses SET ";

        std::vector<std::string> sets;
        
        if (!obj.buildingNo.empty()) {
            sets.push_back("building_no = '" + conn->escape(obj.buildingNo) + "'");
        }
        if (!obj.unitNo.empty()) {
            sets.push_back("unit_no = '" + conn->escape(obj.unitNo) + "'");
        }
        if (!obj.roomNo.empty()) {
            sets.push_back("room_no = '" + conn->escape(obj.roomNo) + "'");
        }
        sets.push_back("area = " + std::to_string(obj.area));
        sets.push_back("owner_id = " + std::to_string(obj.ownerId));

        if (sets.empty()) {
            LOG_WARN << "updateHouse: 没有需要更新的字段";
            return false;
        }

        for (size_t i = 0; i < sets.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sets[i];
        }
        oss << " WHERE id = " << obj.id;

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "更新House失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updateHouse 异常: " << e.what();
        return false;
    }
}

// ========== 删除 ==========
bool HouseService::deleteHouse(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "DELETE FROM houses WHERE id = " + std::to_string(id);
        auto result = conn->execute(sql);
        
        if (!result.success) {
            LOG_ERROR << "删除House失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "deleteHouse 异常: " << e.what();
        return false;
    }
}

// ========== 分页查询 ==========
std::vector<models::House> HouseService::getHousesByPage(int page, int pageSize, int& total,
                                                          const std::string& keyword,
                                                          const std::string& filter1,
                                                          const std::string& filter2) {
    std::vector<models::House> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 构建 WHERE 条件
        std::vector<std::string> whereParts;
        if (!keyword.empty()) {
            std::string safeKw = conn->escape(keyword);
            whereParts.push_back("building_no LIKE '%" + safeKw + "%'");
        }
        if (!filter1.empty()) {
            std::string safe = conn->escape(filter1);
            whereParts.push_back("owner_id = '" + safe + "'");
        }
        
        std::string whereClause = "";
        if (!whereParts.empty()) {
            whereClause = " WHERE " + whereParts[0];
            for (size_t i = 1; i < whereParts.size(); ++i) {
                whereClause += " AND " + whereParts[i];
            }
        }

        // 查询总数
        std::string countSql = "SELECT COUNT(*) as cnt FROM houses" + whereClause;
        auto countResult = conn->query(countSql);
        if (countResult.success && !countResult.rows.empty()) {
            auto it = countResult.rows[0].find("cnt");
            if (it != countResult.rows[0].end()) {
                total = std::stoi(it->second);
            }
        }

        // 分页查询
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT id, building_no, unit_no, room_no, area, owner_id FROM houses" + whereClause
                          + " ORDER BY id LIMIT " 
                          + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
        
        auto result = conn->query(sql);
        if (!result.success) {
            LOG_ERROR << "分页查询House失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::House::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getHousesByPage 异常: " << e.what();
        total = 0;
    }

    return list;
}

}  // namespace services
}  // namespace community
