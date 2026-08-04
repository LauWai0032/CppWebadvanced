// services/IUserService.h
#pragma once
#include "../models/User.h"
#include <vector>
#include <memory>

namespace services
{
  // 抽象基类，定义业务接口
  class IUserService
  {
  public:
    virtual ~IUserService() = default;

    // 纯虚函数，强制子类实现
    virtual std::vector<models::User> getAllUsers() = 0;
    virtual bool authenticate(const std::string &user, const std::string &pass) = 0;
  };
}