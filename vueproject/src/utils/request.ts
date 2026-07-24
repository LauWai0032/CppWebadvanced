import axios from 'axios'
// 引入 Axios 的核心类型定义
import type { AxiosInstance, InternalAxiosRequestConfig, AxiosResponse } from 'axios'
import { ElMessage } from 'element-plus'

// 1. 定义后端统一返回的数据结构泛型
// T 代表 data 字段的具体类型，默认是 any
interface BaseResponse<T = any> {
  code: number
  msg: string
  data: T
}

// 2. 创建 axios 实例
const service: AxiosInstance = axios.create({
  baseURL: 'http://127.0.0.1:8080', // 基础URL
  timeout: 10000 // 超时时间 10秒
})

// 3. 请求拦截器：在发送请求之前做些什么
service.interceptors.request.use(
  (config: InternalAxiosRequestConfig) => {
    const token = localStorage.getItem('token')
    // 如果有 token，添加到请求头
    if (token) {
      // 注意：headers 类型可能较严格，有时需要 as any 或具体类型断言
      config.headers.Authorization = `Bearer ${token}`
    }
    // 设置默认内容类型
    config.headers['Content-Type'] = 'application/json'
    
    // 【重点】必须返回 config，否则请求发不出去
    return config
  },
  (error) => {
    // 处理请求错误（例如请求被取消）
    console.error('Request Error:', error)
    return Promise.reject(error)
  }
)

// 4. 响应拦截器：处理后端返回的数据
// 注意：这里我们将返回类型定义为 AxiosResponse<BaseResponse>
service.interceptors.response.use(
  // 这里的 (response) => ... 是成功回调
  // 我们添加 <any> 来绕过 TS 对返回值的严格检查，因为我们要改变返回结构
  (response: AxiosResponse<BaseResponse>) => {
    const res = response.data

    // 假设 code 200 为业务成功
    if (res.code === 200) {
      // 【优化点】：这里直接返回 res.data
      // 这样你在组件里调用 api.getUser() 拿到的就是用户数据，而不是整个包裹层
      // 注意：这里必须加 as any，否则 TS 会报错说 "BaseResponse" 缺少 "status" 等属性
      return res.data as any
    } else {
      // 业务错误处理 (例如 token 过期、参数错误)
      ElMessage.error(res.msg || 'Error')

      // 特殊状态码处理，例如 401 未授权
      if (res.code === 401) {
        // 执行登出逻辑...
        // window.location.href = '/login'
      }

      // 【关键点】：错误情况必须 reject，且最好抛出 error 对象或自定义 Error
      // 不要直接 return res，否则组件里的 .catch 捕获不到错误
      return Promise.reject(new Error(res.msg || 'Error'))
    }
  },

  // 这里的 (error) => ... 是 HTTP 请求失败回调 (如 404, 500, 断网)
  (error) => {
    console.log('err' + error) // for debug
    ElMessage.error(error.message || '网络请求异常')

    // 必须 reject，让调用者的 catch 能够捕获
    return Promise.reject(error)
  }
)
export default service