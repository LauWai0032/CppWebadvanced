CREATE TABLE service_requests (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL COMMENT '发起人ID',
    type ENUM('repair', 'complaint', 'suggestion') NOT NULL COMMENT '类型：报修/投诉/建议',
    title VARCHAR(100) NOT NULL COMMENT '标题',
    description TEXT COMMENT '详细描述',
    status ENUM('pending', 'processing', 'done') DEFAULT 'pending' COMMENT '状态：待处理/处理中/已完成',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
) COMMENT = '服务请求表';