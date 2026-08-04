<template>
  <div class="home-container">
    <!-- 顶部导航栏 -->
    <el-header class="header">
      <div class="logo-area">
        <span class="logo-icon">🏢</span>
        <h3>社区物业后台管理系统</h3>
      </div>
      <div class="user-info">
        <span class="welcome-text">欢迎，{{ userInfo?.real_name || '管理员' }}</span>
        <el-button type="danger" link @click="logout">退出登录</el-button>
      </div>
    </el-header>

    <!-- 主体区域 -->
    <el-container class="main-container">
      <!-- 左侧侧边栏 -->
      <el-aside width="220px" class="aside">
        <el-menu
          default-active="1"
          class="el-menu-vertical"
          background-color="#304156"
          text-color="#bfcbd9"
          active-text-color="#409EFF"
          router
        >
          <el-menu-item index="/home">
            <el-icon><House /></el-icon>
            <span>首页概览</span>
          </el-menu-item>

          <el-sub-menu index="2">
            <template #title>
              <el-icon><User /></el-icon>
              <span>用户管理</span>
            </template>
            <el-menu-item index="/users/owners">业主列表</el-menu-item>
            <el-menu-item index="/users/staff">物业/管理员</el-menu-item>
          </el-sub-menu>

          <el-sub-menu index="3">
            <template #title>
              <el-icon><OfficeBuilding /></el-icon>
              <span>社区业务</span>
            </template>
            <el-menu-item index="/business/houses">房屋管理</el-menu-item>
            <el-menu-item index="/business/parking">车辆车位</el-menu-item>
            <el-menu-item index="/business/forum">论坛帖子</el-menu-item>
            <el-menu-item index="/business/market">二手交易</el-menu-item>
          </el-sub-menu>

          <el-sub-menu index="4">
            <template #title>
              <el-icon><Wallet /></el-icon>
              <span>物业账单</span>
            </template>
            <el-menu-item index="/bills/payment">缴费记录</el-menu-item>
            <el-menu-item index="/bills/repair">报修投诉</el-menu-item>
          </el-sub-menu>
        </el-menu>
      </el-aside>

      <!-- 右侧内容区 -->
      <el-main class="main-content">
        <router-view v-if="$route.path !== '/home'" />
        <div v-else class="welcome-card">
          <h2>欢迎进入社区管理平台 👋</h2>
          <p>请在左侧菜单栏选择对应的业务功能进行操作。</p>
          <div class="stats-placeholder">
             <!-- 这里以后可以放一些统计卡片 -->
             <el-alert title="系统运行正常" type="success" :closable="false" show-icon />
          </div>
        </div>
      </el-main>
    </el-container>
  </div>
</template>

<script setup>
import { useRouter, useRoute } from 'vue-router'
import { ElMessage, ElMessageBox } from 'element-plus'
import { ref, onMounted } from 'vue'
// 引入图标 (如果你还没安装图标库，请先运行 npm install @element-plus/icons-vue)
import { House, User, OfficeBuilding, Wallet } from '@element-plus/icons-vue'

const router = useRouter()
const route = useRoute()

// 安全获取用户信息，防止报错
const userInfo = ref(JSON.parse(localStorage.getItem('userInfo') || '{}'))

// 退出登录
const logout = () => {
  ElMessageBox.confirm('确定要退出登录吗?', '提示', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    type: 'warning',
  }).then(() => {
    localStorage.removeItem('token')
    localStorage.removeItem('userInfo')
    ElMessage.success('已退出登录')
    router.push('/login')
  }).catch(() => {})
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
  box-shadow: 0 1px 4px rgba(0,21,41,.08);
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
  box-shadow: 0 2px 12px 0 rgba(0,0,0,.1);
  min-height: 300px;
}

.stats-placeholder {
  margin-top: 20px;
}
</style>