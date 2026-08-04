// models/User.h
#pragma once
#include <string>
#include <vector>

namespace models
{
  struct User
  {
    int id;
    std::string username;
    std::string real_name;
    std::string role; // admin, owner, property

    // 默认构造函数
    User() : id(0) {}
  };
}