/**
 * @file main.ts
 * @brief Vue 应用入口文件
 *
 * 创建 Vue 应用实例，注册所有全局插件和组件，然后挂载到 DOM。
 *
 * 注册的全局插件（按顺序）：
 *   1. Pinia - 状态管理（必须在 router 之前，因为路由守卫可能用到 store）
 *   2. Vue Router - 路由管理
 *   3. Element Plus - UI 组件库
 *
 * 设计原则：
 *   - 保持入口文件简洁，复杂的配置拆分到各自的模块文件
 *   - 注册顺序有讲究：Pinia 先于 Router，因为路由守卫可能需要读取 store 状态
 */

import { createApp } from 'vue'
import { createPinia } from 'pinia'
import App from './App.vue'
import router from './router'
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'
import * as ElementPlusIconsVue from '@element-plus/icons-vue'

// ==================== 创建应用实例 ====================
const app = createApp(App)

// ==================== 注册 Pinia 状态管理 ====================
// 必须在路由之前注册，因为路由守卫里可能需要访问 store
const pinia = createPinia()
app.use(pinia)

// ==================== 注册路由 ====================
app.use(router)

// ==================== 注册 Element Plus UI 库 ====================
app.use(ElementPlus)

// ==================== 全局注册 Element Plus 图标组件 ====================
// 将所有图标组件注册为全局组件，方便在模板中直接使用
// 如：<el-icon><House /></el-icon>
for (const [key, component] of Object.entries(ElementPlusIconsVue)) {
  app.component(key, component)
}

// ==================== 挂载应用 ====================
// 挂载到 index.html 中的 <div id="app"></div>
app.mount('#app')
