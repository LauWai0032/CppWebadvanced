/**
 * @file HouseService.h
 * @brief House服务实现类
 *
 * 实现 IHouseService 接口，通过 MySQL 数据库进行实际的数据持久化。
 *
 * @date 2025
 */

#pragma once

#include "IHouseService.h"
#include "common/DbPool.h"
#include "common/Logger.h"

#include <mutex>

namespace community {
namespace services {

/**
 * @class HouseService
 * @brief House服务实现类（MySQL 数据库版）
 *
 * 通过 DbPool 获取数据库连接，执行 SQL 操作。
 */
class HouseService : public IHouseService {
public:
    HouseService() = default;
    ~HouseService() override = default;

    static void setInstance(std::shared_ptr<HouseService> instance);
    static std::shared_ptr<HouseService> getInstance();

    std::vector<models::House> getAllHouses() override;
    std::optional<models::House> getHouseById(int id) override;
    int createHouse(const models::House& obj) override;
    bool updateHouse(const models::House& obj) override;
    bool deleteHouse(int id) override;
    std::vector<models::House> getHousesByPage(int page, int pageSize, int& total,
                                                const std::string& keyword = "",
                                                const std::string& filter1 = "",
                                                const std::string& filter2 = "") override;

private:
    static std::shared_ptr<HouseService> s_instance_;
    static std::mutex s_mutex_;
};

}  // namespace services
}  // namespace community
