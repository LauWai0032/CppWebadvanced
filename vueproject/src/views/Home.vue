<template>
  <div class="home">
    <el-header class="header">
      <h3>社区物业后台管理系统</h3>
      <div class="user-info">
        <span>欢迎，{{ userInfo.real_name }}</span>
        <el-button type="text" @click="logout">退出登录</el-button>
      </div>
    </el-header>
    <el-container style="height: calc(100vh - 60px)">
      <el-aside width="220px" style="background:#304156;">
        <el-menu
          default-active="1"
          class="el-menu-vertical-demo"
          background="#304156"
          text-color="#fff"
          active-text-color="#409EFF"
        >
          <el-menu-item index="1">
            <span>首页概览</span>
          </el-menu-item>
          <el-sub-menu index="2">
            <template #title>用户管理</template>
            <el-menu-item index="2-1">业主列表</el-menu-item>
            <el-menu-item index="2-2">物业/管理员</el-menu-item>
          </el-sub-menu>
          <el-sub-menu index="3">
            <template #title>社区业务</template>
            <el-menu-item index="3-1">房屋管理</el-menu-item>
            <el-menu-item index="3-2">车辆车位</el-menu-item>
            <el-menu-item index="3-3">论坛帖子</el-menu-item>
            <el-menu-item index="3-4">二手交易</el-menu-item>
          </el-sub-menu>
          <el-sub-menu index="4">
            <template #title>物业账单</template>
            <el-menu-item index="4-1">缴费记录</el-menu-item>
            <el-menu-item index="4-2">报修投诉</el-menu-item>
          </el-sub-menu>
        </el-menu>
      </el-aside>
      <el-main>
        <h2>欢迎进入社区管理平台</h2>
        <p>左侧菜单栏可切换对应业务功能页面</p>
      </el-main>
    </el-container>
  </div>
</template>

<script setup>
import { useRouter } from 'vue-router'
import { ElMessage } from 'element-plus'
import { ref } from 'vue'

const router = useRouter()
let userInfo = ref(JSON.parse(localStorage.getItem('userInfo')))

// 退出登录
const logout = () => {
  localStorage.removeItem('token')
  localStorage.removeItem('userInfo')
  ElMessage.success('已退出登录')
  router.push('/login')
}
</script>

<style scoped>
.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
.home {
  height: 100vh;
}
</style>
