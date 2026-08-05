/**
 * @file router/index.ts
 * @brief Vue Router 路由配置
 *
 * 定义应用的所有路由规则和导航守卫。
 *
 * 路由结构：
 *   /          → 重定向到 /home
 *   /login     → 登录页（公开）
 *   /home      → 主框架页（需认证），包含侧边栏和内容区
 *     └─ /users → 用户列表页（嵌套在 Home 的 <router-view> 中）
 *
 * 导航守卫：
 *   - requireAuth: 检查 localStorage 中是否有 Token，没有则重定向到登录页
 *
 * 后续扩展：
 *   - 新增模块时，在 /home 的 children 中添加路由即可
 *   - 可考虑使用路由元信息（meta）控制权限、面包屑等
 */

import { createRouter, createWebHistory } from 'vue-router'
import type { RouteLocationNormalized, NavigationGuardNext } from 'vue-router'
import Login from '../views/Login.vue'
import Home from '../views/Home.vue'
import UserList from '../views/user/UserList.vue'
import HouseList from '../views/house/HouseList.vue'
import VehicleList from '../views/vehicle/VehicleList.vue'
import ParkingSpotList from '../views/parking/ParkingSpotList.vue'
import ForumPostList from '../views/forum/ForumPostList.vue'
import MarketItemList from '../views/marketplace/MarketItemList.vue'
import PropertyFeeList from '../views/property-fee/PropertyFeeList.vue'
import ServiceRequestList from '../views/service-request/ServiceRequestList.vue'

/**
 * @brief 登录认证守卫
 *
 * 检查用户是否已登录（通过 localStorage 中的 token 判断）。
 * 未登录则强制跳转到登录页，并附带 redirect 参数便于登录后返回原页面。
 *
 * 注意：
 *   - 前端仅判断 Token 是否存在，不验证有效性
 *   - 真正的有效性验证由后端 AuthFilter 完成
 *   - 如果后端返回 401，前端应清除 Token 并跳转登录
 *
 * @param to   目标路由
 * @param from 来源路由
 * @param next 放行函数
 */
const requireAuth = (
  to: RouteLocationNormalized,
  from: RouteLocationNormalized,
  next: NavigationGuardNext,
) => {
  const token = localStorage.getItem('token')
  if (token) {
    // 有 token，放行
    next()
  } else {
    // 没 token，强制跳转登录页
    // 携带 redirect 参数，登录成功后可返回原页面
    next({ path: '/login', query: { redirect: to.fullPath } })
  }
}

/**
 * @brief 路由表
 *
 * 使用嵌套路由（children）实现侧边栏布局：
 *   - Home 组件包含侧边栏和 <router-view> 内容区
 *   - 子路由（如 /users）的组件渲染在 Home 的 <router-view> 中
 */
const routes = [
  // 根路径重定向到首页
  { path: '/', redirect: '/home' },

  // 登录页（公开，无需认证）
  { path: '/login', component: Login },

  // 主框架页（需认证）
  {
    path: '/home',
    component: Home,
    beforeEnter: requireAuth,
    // 子路由：内容区会根据子路由切换
    children: [
      {
        path: '/users',
        component: UserList,
        meta: { title: '用户管理' },
      },
      {
        path: '/houses',
        component: HouseList,
        meta: { title: '房屋管理' },
      },
      {
        path: '/vehicles',
        component: VehicleList,
        meta: { title: '车辆管理' },
      },
      {
        path: '/parking',
        component: ParkingSpotList,
        meta: { title: '车位管理' },
      },
      {
        path: '/forum',
        component: ForumPostList,
        meta: { title: '论坛帖子' },
      },
      {
        path: '/market',
        component: MarketItemList,
        meta: { title: '二手交易' },
      },
      {
        path: '/bills',
        component: PropertyFeeList,
        meta: { title: '物业账单' },
      },
      {
        path: '/repair',
        component: ServiceRequestList,
        meta: { title: '报修投诉' },
      },
    ],
  },
]

// ==================== 创建路由实例 ====================
const router = createRouter({
  history: createWebHistory(), // 使用 HTML5 History 模式
  routes,
})

export default router
