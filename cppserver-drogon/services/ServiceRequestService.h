/**
 * @file ServiceRequestService.h
 * @brief ServiceRequest服务实现类
 *
 * 实现 IServiceRequestService 接口，通过 MySQL 数据库进行实际的数据持久化。
 *
 * @date 2025
 */

#pragma once

#include "IServiceRequestService.h"
#include "common/DbPool.h"
#include "common/Logger.h"

#include <mutex>

namespace community {
namespace services {

/**
 * @class ServiceRequestService
 * @brief ServiceRequest服务实现类（MySQL 数据库版）
 *
 * 通过 DbPool 获取数据库连接，执行 SQL 操作。
 */
class ServiceRequestService : public IServiceRequestService {
public:
    ServiceRequestService() = default;
    ~ServiceRequestService() override = default;

    static void setInstance(std::shared_ptr<ServiceRequestService> instance);
    static std::shared_ptr<ServiceRequestService> getInstance();

    std::vector<models::ServiceRequest> getAllServiceRequests() override;
    std::optional<models::ServiceRequest> getServiceRequestById(int id) override;
    int createServiceRequest(const models::ServiceRequest& obj) override;
    bool updateServiceRequest(const models::ServiceRequest& obj) override;
    bool deleteServiceRequest(int id) override;
    std::vector<models::ServiceRequest> getServiceRequestsByPage(int page, int pageSize, int& total,
                                                const std::string& keyword = "",
                                                const std::string& filter1 = "",
                                                const std::string& filter2 = "") override;

    bool updateStatus(int id, const std::string& status) override;

private:
    static std::shared_ptr<ServiceRequestService> s_instance_;
    static std::mutex s_mutex_;
};

}  // namespace services
}  // namespace community
