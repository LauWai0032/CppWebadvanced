CREATE TABLE parking_spots (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '车位ID',
    spot_number VARCHAR(20) NOT NULL UNIQUE COMMENT '车位编号(如 A-001)',
    zone VARCHAR(10) COMMENT '区域(如 A区, B区)',
    type ENUM('fixed', 'temporary') DEFAULT 'temporary' COMMENT '类型：固定车位/临时车位',
    
    -- 关键点：这里引用了上面建好的 vehicles 表
    current_vehicle_id INT COMMENT '当前停放的车辆ID', 
    
    is_occupied TINYINT DEFAULT 0 COMMENT '是否占用：0空闲 1占用',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    -- 建立外键约束
    FOREIGN KEY (current_vehicle_id) REFERENCES vehicles(id)
) COMMENT = '公共停车场车位实时状态表';