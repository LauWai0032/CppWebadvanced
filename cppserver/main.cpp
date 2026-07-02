#include "httplib.h"
#include <iostream>

int main()
{
  // 1. 创建一个 HTTP 服务器实例
  httplib::Server svr;

  // 2. 捕获前端发来的 GET /api/hello 请求
  svr.Get("/api/hello", [](const httplib::Request &req, httplib::Response &res)
          {
        // 3. 返回一个 JSON 格式的数据（与前端 axios 完美对接）
        std::string json_data = R"({
            "code": 200,
            "msg": "Hello from C++ Backend!",
            "data": {
                "server": "cpp-httplib",
                "status": "running"
            }
        })";

        // 4. 设置响应内容，并指定类型为 application/json
        res.set_content(json_data, "application/json"); });

  // 5. 启动服务器，监听 0.0.0.0 的 8080 端口
  std::cout << "C++ Backend is running on http://localhost:8080" << std::endl;
  svr.listen("0.0.0.0", 8080);

  return 0;
}