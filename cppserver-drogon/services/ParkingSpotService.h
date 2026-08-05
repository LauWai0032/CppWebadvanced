/**
 * @file ParkingSpotService.h
 * @brief ParkingSpot服务实现类
 *
 * 实现 IParkingSpotService 接口，通过 MySQL 数据库进行实际的数据持久化。
 *
 * @date 2025
 */

#pragma once

#include "IParkingSpotService.h"
#include "common/DbPool.h"
#include "common/Logger.h"

#include <mutex>

namespace community {
namespace services {

/**
 * @class ParkingSpotService
 * @brief ParkingSpot服务实现类（MySQL 数据库版）
 *
 * 通过 DbPool 获取数据库连接，执行 SQL 操作。
 */
class ParkingSpotService : public IParkingSpotService {
public:
    ParkingSpotService() = default;
    ~ParkingSpotService() override = default;

    static void setInstance(std::shared_ptr<ParkingSpotService> instance);
    static std::shared_ptr<ParkingSpotService> getInstance();

    std::vector<models::ParkingSpot> getAllParkingSpots() override;
    std::optional<models::ParkingSpot> getParkingSpotById(int id) override;
    int createParkingSpot(const models::ParkingSpot& obj) override;
    bool updateParkingSpot(const models::ParkingSpot& obj) override;
    bool deleteParkingSpot(int id) override;
    std::vector<models::ParkingSpot> getParkingSpotsByPage(int page, int pageSize, int& total,
                                                const std::string& keyword = "",
                                                const std::string& filter1 = "",
                                                const std::string& filter2 = "") override;

    bool park(int id, int vehicleId) override;
    bool leave(int id) override;

private:
    static std::shared_ptr<ParkingSpotService> s_instance_;
    static std::mutex s_mutex_;
};

}  // namespace services
}  // namespace community
