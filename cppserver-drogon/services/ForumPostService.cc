/**
 * @file ForumPostService.cc
 * @brief ForumPost服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "ForumPostService.h"
#include <sstream>
#include <cstring>

namespace community {
namespace services {

// ========== 静态成员初始化 ==========
std::shared_ptr<ForumPostService> ForumPostService::s_instance_ = nullptr;
std::mutex ForumPostService::s_mutex_;

void ForumPostService::setInstance(std::shared_ptr<ForumPostService> instance) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_instance_ = std::move(instance);
}

std::shared_ptr<ForumPostService> ForumPostService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_instance_;
}

// ========== 查询所有 ==========
std::vector<models::ForumPost> ForumPostService::getAllForumPosts() {
    std::vector<models::ForumPost> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        auto result = conn->query("SELECT id, user_id, title, content, category, tags, attachments, status, created_at FROM forum_posts ORDER BY id");
        
        if (!result.success) {
            LOG_ERROR << "查询ForumPost列表失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::ForumPost::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getAllForumPosts 异常: " << e.what();
    }

    return list;
}

// ========== 根据 ID 查询 ==========
std::optional<models::ForumPost> ForumPostService::getForumPostById(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "SELECT id, user_id, title, content, category, tags, attachments, status, created_at FROM forum_posts WHERE id = " + std::to_string(id);
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        return models::ForumPost::fromDbRow(result.rows[0]);
    } catch (const std::exception& e) {
        LOG_ERROR << "getForumPostById 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 创建 ==========
int ForumPostService::createForumPost(const models::ForumPost& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        auto safeTitle = conn->escape(obj.title);
        auto safeContent = conn->escape(obj.content);
        auto safeCategory = conn->escape(obj.category);
        auto safeTags = conn->escape(obj.tags);
        auto safeAttachments = conn->escape(obj.attachments);
        auto safeCreatedAt = conn->escape(obj.createdAt);

        std::ostringstream oss;
        oss << "INSERT INTO forum_posts (user_id, title, content, category, tags, attachments, status, created_at) VALUES ("
           << obj.userId
           << ", '" << safeTitle << "'"
           << ", '" << safeContent << "'"
           << ", '" << safeCategory << "'"
           << ", '" << safeTags << "'"
           << ", '" << safeAttachments << "'"
           << ", " << obj.status
           << ", '" << safeCreatedAt << "'"
            << ");";

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "创建ForumPost失败: " << result.error;
            return -1;
        }

        LOG_INFO << "创建ForumPost成功，ID: " << result.insertId;
        return result.insertId;
    } catch (const std::exception& e) {
        LOG_ERROR << "createForumPost 异常: " << e.what();
        return -1;
    }
}

// ========== 更新 ==========
bool ForumPostService::updateForumPost(const models::ForumPost& obj) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        std::ostringstream oss;
        oss << "UPDATE forum_posts SET ";

        std::vector<std::string> sets;
        
        sets.push_back("user_id = " + std::to_string(obj.userId));
        if (!obj.title.empty()) {
            sets.push_back("title = '" + conn->escape(obj.title) + "'");
        }
        if (!obj.content.empty()) {
            sets.push_back("content = '" + conn->escape(obj.content) + "'");
        }
        if (!obj.category.empty()) {
            sets.push_back("category = '" + conn->escape(obj.category) + "'");
        }
        if (!obj.tags.empty()) {
            sets.push_back("tags = '" + conn->escape(obj.tags) + "'");
        }
        if (!obj.attachments.empty()) {
            sets.push_back("attachments = '" + conn->escape(obj.attachments) + "'");
        }
        sets.push_back("status = " + std::to_string(obj.status));
        if (!obj.createdAt.empty()) {
            sets.push_back("created_at = '" + conn->escape(obj.createdAt) + "'");
        }

        if (sets.empty()) {
            LOG_WARN << "updateForumPost: 没有需要更新的字段";
            return false;
        }

        for (size_t i = 0; i < sets.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sets[i];
        }
        oss << " WHERE id = " << obj.id;

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "更新ForumPost失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updateForumPost 异常: " << e.what();
        return false;
    }
}

// ========== 删除 ==========
bool ForumPostService::deleteForumPost(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "DELETE FROM forum_posts WHERE id = " + std::to_string(id);
        auto result = conn->execute(sql);
        
        if (!result.success) {
            LOG_ERROR << "删除ForumPost失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "deleteForumPost 异常: " << e.what();
        return false;
    }
}

// ========== 分页查询 ==========
std::vector<models::ForumPost> ForumPostService::getForumPostsByPage(int page, int pageSize, int& total,
                                                          const std::string& keyword,
                                                          const std::string& filter1,
                                                          const std::string& filter2) {
    std::vector<models::ForumPost> list;
    
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 构建 WHERE 条件
        std::vector<std::string> whereParts;
        if (!keyword.empty()) {
            std::string safeKw = conn->escape(keyword);
            whereParts.push_back("title LIKE '%" + safeKw + "%'");
        }
        if (!filter1.empty()) {
            std::string safe = conn->escape(filter1);
            whereParts.push_back("category = '" + safe + "'");
        }
        
        std::string whereClause = "";
        if (!whereParts.empty()) {
            whereClause = " WHERE " + whereParts[0];
            for (size_t i = 1; i < whereParts.size(); ++i) {
                whereClause += " AND " + whereParts[i];
            }
        }

        // 查询总数
        std::string countSql = "SELECT COUNT(*) as cnt FROM forum_posts" + whereClause;
        auto countResult = conn->query(countSql);
        if (countResult.success && !countResult.rows.empty()) {
            auto it = countResult.rows[0].find("cnt");
            if (it != countResult.rows[0].end()) {
                total = std::stoi(it->second);
            }
        }

        // 分页查询
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT id, user_id, title, content, category, tags, attachments, status, created_at FROM forum_posts" + whereClause
                          + " ORDER BY id LIMIT " 
                          + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
        
        auto result = conn->query(sql);
        if (!result.success) {
            LOG_ERROR << "分页查询ForumPost失败: " << result.error;
            return list;
        }

        for (const auto& row : result.rows) {
            list.push_back(models::ForumPost::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getForumPostsByPage 异常: " << e.what();
        total = 0;
    }

    return list;
}

}  // namespace services
}  // namespace community
