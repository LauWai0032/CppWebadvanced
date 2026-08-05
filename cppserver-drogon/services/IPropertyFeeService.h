/**
 * @file IPropertyFeeService.h
 * @brief PropertyFee服务接口（抽象基类）
 *
 * 定义 PropertyFee相关业务操作的接口。
 * 采用依赖倒置原则：Controller 依赖接口而非具体实现。
 *
 * @date 2025
 */

#pragma once

#include "models/PropertyFee.h"
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace community {
namespace services {

/**
 * @class IPropertyFeeService
 * @brief PropertyFee服务接口
 *
 * 所有 PropertyFee业务操作的抽象接口。
 * 具体实现类必须实现所有纯虚方法。
 */
class IPropertyFeeService {
public:
    virtual ~IPropertyFeeService() = default;

    /**
     * @brief 获取所有列表
     */
    virtual std::vector<models::PropertyFee> getAllPropertyFees() = 0;

    /**
     * @brief 根据 ID 查询
     */
    virtual std::optional<models::PropertyFee> getPropertyFeeById(int id) = 0;

    /**
     * @brief 创建
     */
    virtual int createPropertyFee(const models::PropertyFee& obj) = 0;

    /**
     * @brief 更新
     */
    virtual bool updatePropertyFee(const models::PropertyFee& obj) = 0;

    /**
     * @brief 删除
     */
    virtual bool deletePropertyFee(int id) = 0;

    /**
     * @brief 分页查询
     */
    virtual std::vector<models::PropertyFee> getPropertyFeesByPage(int page, int pageSize, int& total,
                                                        const std::string& keyword = "",
                                                        const std::string& filter1 = "",
                                                        const std::string& filter2 = "",
                                                        const std::string& filter3 = "") = 0;

    /**
     * @brief 缴费（修改状态为 paid + 设置支付方式和时间）
     */
    virtual bool pay(int id, const std::string& paymentMethod) = 0;

};

}  // namespace services
}  // namespace community
