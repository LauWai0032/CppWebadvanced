CREATE TABLE houses (
    id INT AUTO_INCREMENT PRIMARY KEY,
    building_no VARCHAR(10) NOT NULL COMMENT '楼栋号 (如: 1栋)',
    unit_no VARCHAR(10) COMMENT '单元号 (如: 2单元)',
    room_no VARCHAR(10) NOT NULL COMMENT '门牌号 (如: 301)',
    area DECIMAL(10, 2) COMMENT '面积(平米)',
    owner_id INT COMMENT '关联 users 表的id (业主)',
    FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE SET NULL
) COMMENT = '房屋信息表';