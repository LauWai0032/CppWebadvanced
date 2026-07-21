-- 插入用户数据 (密码统一为 123456 的 MD5 值)--
INSERT INTO users (username, password_hash, real_name, phone, role) VALUES --
('zhangsan', 'e10adc3949ba59abbe56e057f20f883e', '张三', '13800138000', 'owner'),
('lisi',     'e10adc3949ba59abbe56e057f20f883e', '李四', '13900139000', 'owner'),
('wangwu',   'e10adc3949ba59abbe56e057f20f883e', '王五', '13700137000', 'property'); --物业

-- 插入房屋测试数据
-- 假设 owner_id: 1=张三, 2=李四, NULL=暂无业主/空置房
INSERT INTO houses (building_no, unit_no, room_no, area, owner_id) VALUES 
('1', '1', '101', 89.50, 1),
('1', '1', '102', 89.50, NULL),
('1', '2', '601', 120.00, 2),
('2', '1', '301', 75.20, NULL),
('2', '1', '302', 75.20, NULL),
('2', '2', '1201', 140.55, NULL);