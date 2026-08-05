/**
 * @file IVehicleService.h
 * @brief Vehicle服务接口（抽象基类）
 *
 * 定义 Vehicle相关业务操作的接口。
 * 采用依赖倒置原则：Controller 依赖接口而非具体实现。
 *
 * @date 2025
 */

#pragma once

#include "models/Vehicle.h"
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace community {
namespace services {

/**
 * @class IVehicleService
 * @brief Vehicle服务接口
 *
 * 所有 Vehicle业务操作的抽象接口。
 * 具体实现类必须实现所有纯虚方法。
 */
class IVehicleService {
public:
    virtual ~IVehicleService() = default;

    /**
     * @brief 获取所有列表
     */
    virtual std::vector<models::Vehicle> getAllVehicles() = 0;

    /**
     * @brief 根据 ID 查询
     */
    virtual std::optional<models::Vehicle> getVehicleById(int id) = 0;

    /**
     * @brief 创建
     */
    virtual int createVehicle(const models::Vehicle& obj) = 0;

    /**
     * @brief 更新
     */
    virtual bool updateVehicle(const models::Vehicle& obj) = 0;

    /**
     * @brief 删除
     */
    virtual bool deleteVehicle(int id) = 0;

    /**
     * @brief 分页查询
     */
    virtual std::vector<models::Vehicle> getVehiclesByPage(int page, int pageSize, int& total,
                                                        const std::string& keyword = "",
                                                        const std::string& filter1 = "",
                                                        const std::string& filter2 = "") = 0;

};

}  // namespace services
}  // namespace community
