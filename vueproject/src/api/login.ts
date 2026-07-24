import service from '@/utils/request' // 引入封装好的 axios 实例

// 定义登录接口的请求参数类型
export interface LoginForm {
  username: string
  password: string
}

// 定义登录接口返回的数据结构
// 这里详细定义了 data 里的字段，比 any 更安全
export interface LoginRes {
  code: number
  msg: string
  data: {
    token: string
    id: number
    username: string
    real_name: string
    role: 'admin' | 'property' | 'owner' // 使用字面量类型限制角色
  }
}

// 定义登录 API 函数
// 使用泛型 <LoginRes> 告诉 axios，我期望返回的数据结构符合 LoginRes 接口
export function loginApi(data: LoginForm) {
  return service.post<LoginRes>('/api/login', data)
}