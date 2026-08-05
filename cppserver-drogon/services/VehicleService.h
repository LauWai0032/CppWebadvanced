/**
 * @file VehicleService.h
 * @brief Vehicle服务实现类
 *
 * 实现 IVehicleService 接口，通过 MySQL 数据库进行实际的数据持久化。
 *
 * @date 2025
 */

#pragma once

#include "IVehicleService.h"
#include "common/DbPool.h"
#include "common/Logger.h"

#include <mutex>

namespace community {
namespace services {

/**
 * @class VehicleService
 * @brief Vehicle服务实现类（MySQL 数据库版）
 *
 * 通过 DbPool 获取数据库连接，执行 SQL 操作。
 */
class VehicleService : public IVehicleService {
public:
    VehicleService() = default;
    ~VehicleService() override = default;

    static void setInstance(std::shared_ptr<VehicleService> instance);
    static std::shared_ptr<VehicleService> getInstance();

    std::vector<models::Vehicle> getAllVehicles() override;
    std::optional<models::Vehicle> getVehicleById(int id) override;
    int createVehicle(const models::Vehicle& obj) override;
    bool updateVehicle(const models::Vehicle& obj) override;
    bool deleteVehicle(int id) override;
    std::vector<models::Vehicle> getVehiclesByPage(int page, int pageSize, int& total,
                                                const std::string& keyword = "",
                                                const std::string& filter1 = "",
                                                const std::string& filter2 = "") override;

private:
    static std::shared_ptr<VehicleService> s_instance_;
    static std::mutex s_mutex_;
};

}  // namespace services
}  // namespace community
