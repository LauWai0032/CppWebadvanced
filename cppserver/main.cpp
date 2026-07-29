#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <string>

// 统一跨域头封装，所有接口共用
void setCorsHeader(httplib::Response &res)
{
    // 允许前端本地开发域名
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type,Authorization");
    // 允许前端读取自定义响应头
    res.set_header("Access-Control-Expose-Headers", "*");
}

int main()
{
    using json = nlohmann::json;
    httplib::Server svr;

    // 处理OPTIONS预检请求（核心，解决跨域拦截）
    svr.Options("/.*", [](const httplib::Request &req, httplib::Response &res)
                {
        setCorsHeader(res);
        res.status = 204; });

    // 测试接口
    svr.Get("/api/hello", [](const httplib::Request &req, httplib::Response &res)
            {
        setCorsHeader(res);
        std::string json_data = R"({
            "code": 200,
            "msg": "Hello from C++ Backend!",
            "data": {
                "server": "cpp-httplib",
                "status": "running"
            }
        })";
        res.set_content(json_data, "application/json"); });

    // 新增登录接口 /api/login 匹配前端请求地址
    svr.Post("/api/login", [](const httplib::Request &req, httplib::Response &res)
             {
        setCorsHeader(res);
        
        std::string ret;
        try {
            // 1. 解析请求体
            json req_json = json::parse(req.body);
            
            // 2. 从解析后的 JSON 对象中获取值
            std::string username = req_json.value("username", "");
            std::string password = req_json.value("password", "");

            // 3. 打印调试信息，这是关键！
            std::cout << ">>> 收到登录请求 <<<" << std::endl;
            std::cout << "原始内容: [" << req.body << "]" << std::endl; 
            std::cout << "解析结果 -> 用户: [" << username << "], 密码: [" << password << "]" << std::endl;

            // 4. 进行比对
            if (username == "admin" && password == "123456") {
                std::cout << ">>> 登录成功！ <<<" << std::endl;
                ret = R"({
                    "code": 200,
                    "msg": "登录成功",
                    "data": {
                        "token": "admin-token-123456",
                        "id": 1,
                        "username": "admin",
                        "real_name": "管理员",
                        "role": "admin"
                    }
                })";
            } else {
                std::cout << ">>> 登录失败：账号或密码不匹配 <<<" << std::endl;
                ret = R"({
                    "code": 400,
                    "msg": "账号或密码错误",
                    "data": null
                })";
            }
        } catch (json::parse_error& e) {
            // 5. 处理 JSON 格式错误
            std::cerr << "!!! JSON 解析错误: " << e.what() << std::endl;
            ret = R"({
                "code": 400,
                "msg": "请求数据格式错误",
                "data": null
            })";
        }

        res.set_content(ret, "application/json"); });

    std::cout << "C++ Backend running on http://0.0.0.0:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}