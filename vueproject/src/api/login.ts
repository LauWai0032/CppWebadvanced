import axios from 'axios'

// 登录接口，后端接收 username / password
export function loginApi(data:any) {
  return axios.post('/api/login', data)
}
