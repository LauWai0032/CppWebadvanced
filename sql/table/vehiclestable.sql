CREATE TABLE IF NOT EXISTS vehicles (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '车辆ID',
    user_id INT NOT NULL COMMENT '车主ID',
    plate_number VARCHAR(20) NOT NULL COMMENT '车牌号',
    brand VARCHAR(50) COMMENT '品牌',
    color VARCHAR(20) COMMENT '颜色',
    status TINYINT DEFAULT 1 COMMENT '状态：1正常 0禁用',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
) COMMENT = '业主车辆信息表';