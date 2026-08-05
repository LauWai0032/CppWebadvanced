/**
 * @file ForumPostService.h
 * @brief ForumPost服务实现类
 *
 * 实现 IForumPostService 接口，通过 MySQL 数据库进行实际的数据持久化。
 *
 * @date 2025
 */

#pragma once

#include "IForumPostService.h"
#include "common/DbPool.h"
#include "common/Logger.h"

#include <mutex>

namespace community {
namespace services {

/**
 * @class ForumPostService
 * @brief ForumPost服务实现类（MySQL 数据库版）
 *
 * 通过 DbPool 获取数据库连接，执行 SQL 操作。
 */
class ForumPostService : public IForumPostService {
public:
    ForumPostService() = default;
    ~ForumPostService() override = default;

    static void setInstance(std::shared_ptr<ForumPostService> instance);
    static std::shared_ptr<ForumPostService> getInstance();

    std::vector<models::ForumPost> getAllForumPosts() override;
    std::optional<models::ForumPost> getForumPostById(int id) override;
    int createForumPost(const models::ForumPost& obj) override;
    bool updateForumPost(const models::ForumPost& obj) override;
    bool deleteForumPost(int id) override;
    std::vector<models::ForumPost> getForumPostsByPage(int page, int pageSize, int& total,
                                                const std::string& keyword = "",
                                                const std::string& filter1 = "",
                                                const std::string& filter2 = "") override;

private:
    static std::shared_ptr<ForumPostService> s_instance_;
    static std::mutex s_mutex_;
};

}  // namespace services
}  // namespace community
