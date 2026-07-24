import { createRouter, createWebHistory } from 'vue-router'
// 引入路由守卫所需的类型
import type { RouteLocationNormalized, NavigationGuardNext } from 'vue-router'
import Login from '../views/Login.vue'
import Home from '../views/Home.vue'

// 定义路由守卫函数
// to: 要去哪, from: 从哪来, next: 放行函数
const requireAuth = (to: RouteLocationNormalized, from: RouteLocationNormalized, next: NavigationGuardNext) => {
  const token = localStorage.getItem('token')
  if (token) {
    // 有 token，放行
    next()
  } else {
    // 没 token，强制跳转登录页
    next('/login')
  }
}

// 路由表
const routes = [
  { path: '/', redirect: '/home' }, // 根路径重定向到 home
  { path: '/login', component: Login }, // 登录页
  { 
    path: '/home', 
    component: Home, 
    beforeEnter: requireAuth // 进入 home 前检查权限
  }
]

// 创建路由实例
const router = createRouter({
  history: createWebHistory(), // 使用 HTML5 History 模式
  routes
})

export default router