CREATE TABLE forum_posts (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '帖子ID',
    user_id INT NOT NULL COMMENT '发帖人ID',
    title VARCHAR(200) COMMENT '帖子标题',
    content TEXT COMMENT '帖子正文',
    category ENUM('chat', 'lost_found', 'notice', 'other') DEFAULT 'chat' COMMENT '板块分类',
    tags JSON COMMENT '标签(如: ["求助", "宠物"])，预留扩展空间',
    attachments JSON COMMENT '附件/图片链接数组，预留扩展空间',
    status TINYINT DEFAULT 1 COMMENT '状态：1正常 0隐藏/删除',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
) COMMENT = '社区论坛帖子表';