/**
 * @file MarketplaceItemService.h
 * @brief MarketplaceItem服务实现类
 *
 * 实现 IMarketplaceItemService 接口，通过 MySQL 数据库进行实际的数据持久化。
 *
 * @date 2025
 */

#pragma once

#include "IMarketplaceItemService.h"
#include "common/DbPool.h"
#include "common/Logger.h"

#include <mutex>

namespace community {
namespace services {

/**
 * @class MarketplaceItemService
 * @brief MarketplaceItem服务实现类（MySQL 数据库版）
 *
 * 通过 DbPool 获取数据库连接，执行 SQL 操作。
 */
class MarketplaceItemService : public IMarketplaceItemService {
public:
    MarketplaceItemService() = default;
    ~MarketplaceItemService() override = default;

    static void setInstance(std::shared_ptr<MarketplaceItemService> instance);
    static std::shared_ptr<MarketplaceItemService> getInstance();

    std::vector<models::MarketplaceItem> getAllMarketplaceItems() override;
    std::optional<models::MarketplaceItem> getMarketplaceItemById(int id) override;
    int createMarketplaceItem(const models::MarketplaceItem& obj) override;
    bool updateMarketplaceItem(const models::MarketplaceItem& obj) override;
    bool deleteMarketplaceItem(int id) override;
    std::vector<models::MarketplaceItem> getMarketplaceItemsByPage(int page, int pageSize, int& total,
                                                const std::string& keyword = "",
                                                const std::string& filter1 = "",
                                                const std::string& filter2 = "") override;

private:
    static std::shared_ptr<MarketplaceItemService> s_instance_;
    static std::mutex s_mutex_;
};

}  // namespace services
}  // namespace community
