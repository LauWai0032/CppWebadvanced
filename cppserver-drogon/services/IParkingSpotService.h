/**
 * @file IParkingSpotService.h
 * @brief ParkingSpot服务接口（抽象基类）
 *
 * 定义 ParkingSpot相关业务操作的接口。
 * 采用依赖倒置原则：Controller 依赖接口而非具体实现。
 *
 * @date 2025
 */

#pragma once

#include "models/ParkingSpot.h"
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace community {
namespace services {

/**
 * @class IParkingSpotService
 * @brief ParkingSpot服务接口
 *
 * 所有 ParkingSpot业务操作的抽象接口。
 * 具体实现类必须实现所有纯虚方法。
 */
class IParkingSpotService {
public:
    virtual ~IParkingSpotService() = default;

    /**
     * @brief 获取所有列表
     */
    virtual std::vector<models::ParkingSpot> getAllParkingSpots() = 0;

    /**
     * @brief 根据 ID 查询
     */
    virtual std::optional<models::ParkingSpot> getParkingSpotById(int id) = 0;

    /**
     * @brief 创建
     */
    virtual int createParkingSpot(const models::ParkingSpot& obj) = 0;

    /**
     * @brief 更新
     */
    virtual bool updateParkingSpot(const models::ParkingSpot& obj) = 0;

    /**
     * @brief 删除
     */
    virtual bool deleteParkingSpot(int id) = 0;

    /**
     * @brief 分页查询
     */
    virtual std::vector<models::ParkingSpot> getParkingSpotsByPage(int page, int pageSize, int& total,
                                                        const std::string& keyword = "",
                                                        const std::string& filter1 = "",
                                                        const std::string& filter2 = "") = 0;

    /**
     * @brief 车辆入场（设置当前车辆 + 标记占用）
     */
    virtual bool park(int id, int vehicleId) = 0;

    /**
     * @brief 车辆离场（清空当前车辆 + 标记空闲）
     */
    virtual bool leave(int id) = 0;

};

}  // namespace services
}  // namespace community
