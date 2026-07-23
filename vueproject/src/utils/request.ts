import axios, { AxiosInstance, AxiosRequestConfig, AxiosResponse } from 'axios'
import { ElMessage } from 'element-plus'

// 统一后端返回基础类型
export interface BaseResponse<T = any> {
  code: number
  msg: string
  data: T
}

// 创建axios实例
const service: AxiosInstance = axios.create({
  baseURL: 'http://127.0.0.1:8080',
  timeout: 10000
})

// 请求拦截器
service.interceptors.request.use(
  (config: AxiosRequestConfig) => {
    const token = localStorage.getItem('token')
    if (token && config.headers) {
      config.headers.Authorization = `Bearer ${token}`
    }
    if (config.headers) {
      config.headers['Content-Type'] = 'application/json'
    }
    return config
  },
  (err) => Promise.reject(err)
)

// 响应拦截器
service.interceptors.response.use(
  (response: AxiosResponse<BaseResponse>) => {
    const res = response.data
    switch (res.code) {
      case 200:
        return res
      case 400:
        ElMessage.warning(res.msg || '参数错误，请检查输入')
        return Promise.reject(res)
      case 401:
        ElMessage.warning('登录已失效，请重新登录')
        localStorage.clear()
        location.href = '/login'
        return Promise.reject(res)
      case 403:
        ElMessage.error('无权限访问该功能')
        return Promise.reject(res)
      case 500:
        ElMessage.error('服务器业务异常：' + (res.msg || '未知错误'))
        return Promise.reject(res)
      default:
        ElMessage.error(res.msg || '操作失败')
        return Promise.reject(res)
    }
  },
  (error) => {
    if (error.response) {
      const httpCode = error.response.status
      switch (httpCode) {
        case 404:
          ElMessage.error('接口地址不存在，请检查后端路由')
          break
        case 403:
          ElMessage.error('服务器拒绝访问')
          break
        case 405:
          ElMessage.error('请求方式错误（GET/POST不匹配）')
          break
        case 500:
          ElMessage.error('C++服务内部出错')
          break
        case 503:
          ElMessage.error('服务未启动或不可用')
          break
        default:
          ElMessage.error(`请求失败，HTTP状态码：${httpCode}`)
      }
    } else if (error.message.includes('timeout')) {
      ElMessage.error('请求超时，请检查后端服务是否正常运行')
    } else {
      ElMessage.error('无法连接后端服务器，请确认服务已启动')
    }
    return Promise.reject(error)
  }
)

export default service
