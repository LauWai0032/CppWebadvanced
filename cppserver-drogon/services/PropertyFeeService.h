/**
 * @file PropertyFeeService.h
 * @brief PropertyFee服务实现类
 *
 * 实现 IPropertyFeeService 接口，通过 MySQL 数据库进行实际的数据持久化。
 *
 * @date 2025
 */

#pragma once

#include "IPropertyFeeService.h"
#include "common/DbPool.h"
#include "common/Logger.h"

#include <mutex>

namespace community {
namespace services {

/**
 * @class PropertyFeeService
 * @brief PropertyFee服务实现类（MySQL 数据库版）
 *
 * 通过 DbPool 获取数据库连接，执行 SQL 操作。
 */
class PropertyFeeService : public IPropertyFeeService {
public:
    PropertyFeeService() = default;
    ~PropertyFeeService() override = default;

    static void setInstance(std::shared_ptr<PropertyFeeService> instance);
    static std::shared_ptr<PropertyFeeService> getInstance();

    std::vector<models::PropertyFee> getAllPropertyFees() override;
    std::optional<models::PropertyFee> getPropertyFeeById(int id) override;
    int createPropertyFee(const models::PropertyFee& obj) override;
    bool updatePropertyFee(const models::PropertyFee& obj) override;
    bool deletePropertyFee(int id) override;
    std::vector<models::PropertyFee> getPropertyFeesByPage(int page, int pageSize, int& total,
                                                const std::string& keyword = "",
                                                const std::string& filter1 = "",
                                                const std::string& filter2 = "",
                                                const std::string& filter3 = "") override;

    bool pay(int id, const std::string& paymentMethod) override;

private:
    static std::shared_ptr<PropertyFeeService> s_instance_;
    static std::mutex s_mutex_;
};

}  // namespace services
}  // namespace community
