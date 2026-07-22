-- 插入用户数据 (密码统一为 123456 的 MD5 值：e10adc3949ba59abbe56e057f20f883e)
INSERT INTO users (username, password_hash, real_name, phone, role) VALUES 
('zhangsan', 'e10adc3949ba59abbe56e057f20f883e', '张三', '13800138000', 'owner'),
('lisi',     'e10adc3949ba59abbe56e057f20f883e', '李四', '13900139000', 'owner'),
('wangwu',   'e10adc3949ba59abbe56e057f20f883e', '王五', '13700137000', 'property');

-- 新增管理员账号（适配后台管理测试）
INSERT INTO users (username, password_hash, real_name, phone, role, status) VALUES 
('admin', 'e10adc3949ba59abbe56e057f20f883e', '系统管理员', '13600136000', 'admin', 1);

-- 插入房屋测试数据
-- 假设 owner_id: 1=张三, 2=李四, NULL=暂无业主/空置房
INSERT INTO houses (building_no, unit_no, room_no, area, owner_id) VALUES 
('1', '1', '101', 89.50, 1),
('1', '1', '102', 89.50, NULL),
('1', '2', '601', 120.00, 2),
('2', '1', '301', 75.20, NULL),
('2', '1', '302', 75.20, NULL),
('2', '2', '1201', 140.55, NULL);

-- 车辆数据：张三、李四绑定私人车辆，适配车位停放测试
INSERT INTO vehicles (user_id, plate_number, brand, color, status) VALUES
(1, '粤A12345', '丰田', '白色', 1),
(2, '粤A67890', '本田', '黑色', 1);

-- 覆盖闲聊、失物招领、公告、其他四大板块，包含不同状态、标签
INSERT INTO forum_posts (user_id, title, content, category, tags, attachments, status) VALUES
(1, '小区楼下便利店营业时间', '想问下小区楼下便利店日常营业时间，周末是否营业？', 'chat', '["生活咨询"]', '[]', 1),
(2, '捡到一把家门钥匙', '1栋单元门口捡到一把钥匙，有丢失的业主联系我', 'lost_found', '["失物招领"]', '[]', 1),
(3, '小区停水通知', '物业通知：本周四上午9点-12点小区全域停水检修管道', 'notice', '["小区公告","停水通知"]', '[]', 1),
(1, '闲置健身器材求助', '有没有业主懂健身器材维修的，家里器材有点故障', 'other', '["求助"]', '[]', 0);

-- 包含出售、以物易物两种模式，在售、已售出、已下架三种状态
INSERT INTO marketplace_items (seller_id, title, description, exchange_type, price, expect_item, status) VALUES
(1, '九成新家用微波炉', '使用一年，功能完好，无破损，支持自提', 'sell', 150.00, '', 'active'),
(2, '闲置儿童绘本套装', '全新未拆封绘本，想换家用小台灯', 'barter', 0.00, '家用护眼小台灯', 'active'),
(1, '二手电动车', '续航正常，无维修记录，已售出', 'sell', 800.00, '', 'sold'),
(2, '闲置收纳箱', '闲置收纳箱，暂时下架不出售', 'sell', 30.00, '', 'closed');

-- 覆盖待处理、处理中、已完成三种流程状态
INSERT INTO service_requests (user_id, type, title, description, status) VALUES
(1, 'repair', '卫生间漏水报修', '1栋101卫生间顶部轻微漏水，需要物业上门检修', 'processing'),
(2, 'complaint', '小区夜间噪音过大', '小区广场夜间有人喧哗，影响休息，希望物业管控', 'pending'),
(1, 'suggestion', '建议增设休闲座椅', '建议小区绿化带周边增设休闲座椅，方便业主休息', 'done');