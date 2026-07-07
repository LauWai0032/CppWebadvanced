CREATE TABLE marketplace_items (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '物品ID',
    seller_id INT NOT NULL COMMENT '发布者ID',
    title VARCHAR(150) NOT NULL COMMENT '物品标题',
    description TEXT COMMENT '物品描述',
    exchange_type ENUM('sell', 'barter') DEFAULT 'sell' COMMENT '交易方式：sell(出售) / barter(以物易物)',
    price DECIMAL(10, 2) COMMENT '售价(如果是换物，可设为0或象征性价格)',
    expect_item VARCHAR(200) COMMENT '期望交换的物品(仅barter模式有效，如: 想要一台微波炉)',
    status ENUM('active', 'sold', 'closed') DEFAULT 'active' COMMENT '状态：在售/已售出/已下架',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (seller_id) REFERENCES users(id)
) COMMENT = '二手交易与以物易物表';