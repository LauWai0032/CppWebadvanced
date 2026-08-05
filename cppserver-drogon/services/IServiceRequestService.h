/**
 * @file IServiceRequestService.h
 * @brief ServiceRequest服务接口（抽象基类）
 *
 * 定义 ServiceRequest相关业务操作的接口。
 * 采用依赖倒置原则：Controller 依赖接口而非具体实现。
 *
 * @date 2025
 */

#pragma once

#include "models/ServiceRequest.h"
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace community {
namespace services {

/**
 * @class IServiceRequestService
 * @brief ServiceRequest服务接口
 *
 * 所有 ServiceRequest业务操作的抽象接口。
 * 具体实现类必须实现所有纯虚方法。
 */
class IServiceRequestService {
public:
    virtual ~IServiceRequestService() = default;

    /**
     * @brief 获取所有列表
     */
    virtual std::vector<models::ServiceRequest> getAllServiceRequests() = 0;

    /**
     * @brief 根据 ID 查询
     */
    virtual std::optional<models::ServiceRequest> getServiceRequestById(int id) = 0;

    /**
     * @brief 创建
     */
    virtual int createServiceRequest(const models::ServiceRequest& obj) = 0;

    /**
     * @brief 更新
     */
    virtual bool updateServiceRequest(const models::ServiceRequest& obj) = 0;

    /**
     * @brief 删除
     */
    virtual bool deleteServiceRequest(int id) = 0;

    /**
     * @brief 分页查询
     */
    virtual std::vector<models::ServiceRequest> getServiceRequestsByPage(int page, int pageSize, int& total,
                                                        const std::string& keyword = "",
                                                        const std::string& filter1 = "",
                                                        const std::string& filter2 = "") = 0;

    /**
     * @brief 修改状态（pending → processing → done）
     */
    virtual bool updateStatus(int id, const std::string& status) = 0;

};

}  // namespace services
}  // namespace community
