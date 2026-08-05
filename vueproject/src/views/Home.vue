<!--
  @file views/Home.vue
  @brief 主框架页面（带侧边栏布局）

  页面整体结构：
    ┌────────── 顶部导航栏 ──────────┐
    │  Logo/系统名称        用户信息/退出  │
    ├────────┬───────────────────────┤
    │        │                       │
    │ 侧边栏  │      内容区（router-view） │
    │  菜单   │                       │
    │        │                       │
    └────────┴───────────────────────┘

  设计思路：
    - 使用 Element Plus 的 Container 布局组件
    - 左侧 el-menu 配合 Vue Router 的 router 属性实现菜单路由联动
    - 当前路由路径（$route.path）与菜单 index 匹配，自动高亮
    - 首页概览直接在主内容区显示，子路由则通过 <router-view> 渲染
-->

<template>
  <div class="home-container">
    <!-- ==================== 顶部导航栏 ==================== -->
    <el-header class="header">
      <div class="logo-area">
        <span class="logo-icon">🏢</span>
        <h3>社区物业后台管理系统</h3>
      </div>
      <div class="user-info">
        <span class="welcome-text">欢迎，{{ userInfo?.real_name || userInfo?.username || '管理员' }}</span>
        <el-button type="danger" link @click="logout">退出登录</el-button>
      </div>
    </el-header>

    <!-- ==================== 主体区域 ==================== -->
    <el-container class="main-container">
      <!-- 左侧侧边栏菜单 -->
      <el-aside width="220px" class="aside">
        <!--
          el-menu 的 router 属性：启用后点击菜单项会自动调用 router.push()
          default-active：当前激活菜单项的 index，绑定 $route.path 实现自动高亮
        -->
        <el-menu
          :default-active="activeMenu"
          class="el-menu-vertical"
          background-color="#304156"
          text-color="#bfcbd9"
          active-text-color="#409EFF"
          router
        >
          <!-- 首页 -->
          <el-menu-item index="/home">
            <el-icon><House /></el-icon>
            <span>首页概览</span>
          </el-menu-item>

          <!-- 用户管理（子菜单） -->
          <el-sub-menu index="user-manage">
            <template #title>
              <el-icon><User /></el-icon>
              <span>用户管理</span>
            </template>
            <!--
              注意：index 使用 /users，与路由配置一致
              点击后通过 el-menu 的 router 属性自动跳转
            -->
            <el-menu-item index="/users">用户列表</el-menu-item>
          </el-sub-menu>

          <!-- 社区业务（预留菜单项，后续模块逐步实现） -->
          <el-sub-menu index="business">
            <template #title>
              <el-icon><OfficeBuilding /></el-icon>
              <span>社区业务</span>
            </template>
            <el-menu-item index="/business/houses" disabled>房屋管理</el-menu-item>
            <el-menu-item index="/business/parking" disabled>车辆车位</el-menu-item>
            <el-menu-item index="/business/forum" disabled>论坛帖子</el-menu-item>
            <el-menu-item index="/business/market" disabled>二手交易</el-menu-item>
          </el-sub-menu>

          <!-- 物业账单（预留菜单项） -->
          <el-sub-menu index="bills">
            <template #title>
              <el-icon><Wallet /></el-icon>
              <span>物业账单</span>
            </template>
            <el-menu-item index="/bills/payment" disabled>缴费记录</el-menu-item>
            <el-menu-item index="/bills/repair" disabled>报修投诉</el-menu-item>
          </el-sub-menu>
        </el-menu>
      </el-aside>

      <!-- 右侧内容区 -->
      <el-main class="main-content">
        <!--
          根据当前路由决定显示内容：
            - 如果在 /home 首页，显示欢迎卡片
            - 如果在子路由页面，通过 <router-view> 渲染子组件
        -->
        <router-view v-if="$route.path !== '/home'" />
        <div v-else class="welcome-card">
          <h2>欢迎进入社区管理平台 👋</h2>
          <p>请在左侧菜单栏选择对应的业务功能进行操作。</p>
          <div class="stats-placeholder">
            <el-alert title="系统运行正常" type="success" :closable="false" show-icon />
          </div>
        </div>
      </el-main>
    </el-container>
  </div>
</template>

<script setup lang="ts">
/**
 * @brief 主框架页 - 脚本部分
 *
 * 主要功能：
 *   1. 从 localStorage 读取用户信息（顶部展示）
 *   2. 处理退出登录逻辑
 *   3. 计算当前激活的菜单项
 */

import { useRouter, useRoute } from 'vue-router'
import { ElMessage, ElMessageBox } from 'element-plus'
import { ref, computed } from 'vue'
// Element Plus 图标（已在 main.ts 中全局注册，此处导入用于类型检查）
import { House, User, OfficeBuilding, Wallet } from '@element-plus/icons-vue'

const router = useRouter()
const route = useRoute()

/**
 * @brief 当前登录用户信息
 *
 * 从 localStorage 读取并解析。
 * 注意：这是一个 ref 对象，页面初始加载时读取一次。
 * 如果后续需要响应式更新，应改用 Pinia store。
 *
 * 数据格式（与登录接口返回的 data 一致）：
 *   { id, username, real_name, role }
 */
const userInfo = ref(() => {
  try {
    const stored = localStorage.getItem('userInfo')
    return stored ? JSON.parse(stored) : null
  } catch {
    return null
  }
})

/**
 * @brief 当前激活的菜单项
 *
 * 用于 el-menu 的 default-active 属性。
 * 如果在首页（/home），高亮首页菜单项；
 * 否则高亮当前路由路径对应的菜单项。
 */
const activeMenu = computed(() => {
  if (route.path === '/home') return '/home'
  return route.path
})

/**
 * @brief 退出登录
 *
 * 弹出确认框，用户确认后：
 *   1. 清除 localStorage 中的 token 和用户信息
 *   2. 跳转到登录页
 */
const logout = () => {
  ElMessageBox.confirm('确定要退出登录吗?', '提示', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    type: 'warning',
  })
    .then(() => {
      localStorage.removeItem('token')
      localStorage.removeItem('userInfo')
      ElMessage.success('已退出登录')
      router.push('/login')
    })
    .catch(() => {
      // 用户取消，什么也不做
    })
}
</script>

<style scoped>
/* 整体布局：全屏高度，隐藏滚动条 */
.home-container {
  height: 100vh;
  display: flex;
  flex-direction: column;
  background-color: #f0f2f5; /* 浅灰色背景，护眼 */
}

/* 顶部 Header */
.header {
  background-color: #fff;
  box-shadow: 0 1px 4px rgba(0, 21, 41, 0.08);
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0 20px;
  z-index: 10;
}

.logo-area {
  display: flex;
  align-items: center;
  gap: 10px;
  color: #304156;
}

.logo-area h3 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
}

.user-info {
  display: flex;
  align-items: center;
  gap: 15px;
}

.welcome-text {
  font-size: 14px;
  color: #606266;
}

/* 主体区域 */
.main-container {
  flex: 1; /* 占据剩余高度 */
  overflow: hidden; /* 防止出现双重滚动条 */
}

/* 左侧侧边栏 */
.aside {
  background-color: #304156;
  overflow-y: auto; /* 菜单过长时允许滚动 */
}

/* 去除菜单默认的右边框 */
.el-menu-vertical {
  border-right: none;
  height: 100%;
}

/* 右侧内容区 */
.main-content {
  background-color: #f0f2f5;
  padding: 20px;
  overflow-y: auto;
}

.welcome-card {
  background: #fff;
  padding: 40px;
  border-radius: 4px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
  min-height: 300px;
}

.stats-placeholder {
  margin-top: 20px;
}
</style>
