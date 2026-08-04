// services/UserService.hpp
#pragma once
#include "../models/User.hpp"
#include "./IUserService.hpp"

namespace services
{
  class UserService : public IUserService
  {
  public:
    std::vector<models::User> getAllUsers() override;
    bool authenticate(const std::string &user, const std::string &pass) override;
  };
}