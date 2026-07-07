CREATE TABLE property_fees (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '账单ID',
    house_id INT NOT NULL COMMENT '关联房屋ID',
    fee_type ENUM('property', 'water', 'electricity', 'gas') NOT NULL COMMENT '费用类型',
    amount DECIMAL(10, 2) NOT NULL COMMENT '应缴金额',
    bill_month VARCHAR(7) NOT NULL COMMENT '账单月份(如: 2026-07)',
    status ENUM('unpaid', 'paid', 'overdue') DEFAULT 'unpaid' COMMENT '状态：未缴/已缴/逾期',
    payment_method ENUM('wechat', 'alipay', 'cash', 'bank') COMMENT '支付方式(预留扩展)',
    paid_at TIMESTAMP COMMENT '实际支付时间',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (house_id) REFERENCES houses(id)
) COMMENT = '物业费与账单表';