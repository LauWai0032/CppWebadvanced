-- 1. 用户登录查询
SELECT * FROM users WHERE username = ? AND password_hash = ? AND status = 1;

-- 2. 查询业主绑定房屋
SELECT h.* FROM houses h LEFT JOIN users u ON h.owner_id = u.id WHERE u.username = ?;

-- 3. 查询业主绑定车辆
SELECT * FROM vehicles WHERE user_id = ? AND status = 1;

-- 4. 查询在售二手商品
SELECT * FROM marketplace_items WHERE status = 'active' ORDER BY created_at DESC;

-- 5. 查询未缴物业费账单
SELECT * FROM property_fees WHERE status = 'unpaid' OR status = 'overdue';

-- 6. 查询用户发布的论坛帖子
SELECT * FROM forum_posts WHERE user_id = ? AND status = 1;

-- 7. 查询车位占用状态
SELECT * FROM parking_spots WHERE is_occupied = 1;