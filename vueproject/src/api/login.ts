import service from '@/utils/request'

export interface LoginForm {
  username: string
  password: string
}

export interface LoginRes {
  code: number
  msg: string
  data: {
    token: string
    id: number
    username: string
    real_name: string
    role: 'admin' | 'property' | 'owner'
  }
}

// 写法A：去掉函数返回类型注解，TS自动推导（最简单，无报错）
export function loginApi(data: LoginForm) {
  return service.post<LoginRes>('/api/login', data)
}
