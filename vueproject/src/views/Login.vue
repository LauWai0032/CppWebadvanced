<!--
  @file views/Login.vue
  @brief 用户登录页面

  提供用户名 + 密码登录功能。登录成功后：
    1. 将 Token 和用户信息存入 Pinia store（自动持久化到 localStorage）
    2. 跳转到首页或 redirect 参数指定的页面

  设计要点：
    - 使用 Element Plus Form 组件，包含表单校验
    - 使用 Pinia store 管理用户登录状态
    - 错误处理完善：网络错误和业务错误分别处理
    - 支持 URL redirect 参数（登录后跳回原页面）
-->

<template>
  <div class="login-wrap">
    <div class="login-box">
      <h2 class="title">社区物业管理平台</h2>
      <!--
        el-form 的 :model 绑定表单数据对象
        :rules 绑定校验规则
        ref 用于手动调用 validate 方法
      -->
      <el-form ref="loginFormRef" :model="loginForm" :rules="loginRules" label-width="80px">
        <el-form-item label="账号" prop="username">
          <el-input v-model="loginForm.username" placeholder="请输入账号"></el-input>
        </el-form-item>
        <el-form-item label="密码" prop="password">
          <el-input
            v-model="loginForm.password"
            show-password
            placeholder="请输入密码"
            @keyup.enter="handleLogin"
          ></el-input>
        </el-form-item>
        <el-form-item>
          <!--
            loading 绑定防止重复点击提交
          -->
          <el-button type="primary" class="login-btn" :loading="loading" @click="handleLogin">
            登录
          </el-button>
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>

<script setup lang="ts">
/**
 * @brief 登录页 - 脚本部分
 *
 * 登录流程：
 *   1. 用户输入用户名密码，点击登录（或回车）
 *   2. 前端表单校验（非空等）
 *   3. 调用登录 API
 *   4. 成功：保存 Token 和用户信息到 Pinia，跳转首页
 *   5. 失败：显示错误提示
 *
 * 修复点：
 *   - 原代码 catch 中使用了 res.msg，但 res 在 catch 作用域中不存在
 *   - 正确做法：catch 的参数是 Error 对象，使用 error.message 获取错误信息
 */

import { ref, reactive } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { ElMessage, type FormInstance, type FormRules } from 'element-plus'
import { loginApi } from '@/api/login'
import { useUserStore } from '@/stores/user'

const router = useRouter()
const route = useRoute()
const userStore = useUserStore()

/** 表单引用（用于手动触发校验） */
const loginFormRef = ref<FormInstance>()

/** 登录按钮 loading 状态 */
const loading = ref(false)

/**
 * @brief 登录表单数据
 *
 * 与表单输入项双向绑定。
 */
const loginForm = reactive({
  username: '',
  password: '',
})

/**
 * @brief 登录表单校验规则
 *
 * 用户名和密码均为必填。
 */
const loginRules: FormRules = {
  username: [
    { required: true, message: '请输入账号', trigger: 'blur' },
  ],
  password: [
    { required: true, message: '请输入密码', trigger: 'blur' },
  ],
}

/**
 * @brief 处理登录
 *
 * 步骤：
 *   1. 表单校验（Element Plus validate）
 *   2. 调用登录 API
 *   3. 成功后存入 Pinia store
 *   4. 跳转到首页或 redirect 参数指定的页面
 *
 * 错误处理：
 *   - 业务错误（如密码错误）：API 拦截器已通过 ElMessage.error 提示
 *   - 网络错误：catch 中捕获并提示
 *
 * 【重要修复】：原代码 catch 里写了 `res.msg`，但 res 在 catch 中不存在，
 * 正确的错误信息来自 catch 的参数 error（Error 对象）。
 */
const handleLogin = async () => {
  // 表单校验
  if (!loginFormRef.value) return
  try {
    await loginFormRef.value.validate()
  } catch {
    // 校验不通过，直接返回
    return
  }

  loading.value = true
  try {
    // 调用登录接口
    // request 拦截器会返回 res.data（即 LoginResult 类型）
    const result = await loginApi(loginForm)

    // 保存到 Pinia store（自动持久化到 localStorage）
    userStore.login(result)

    ElMessage.success('登录成功')

    // 跳转：有 redirect 参数则跳回原页面，否则跳首页
    const redirect = route.query.redirect as string | undefined
    router.push(redirect || '/home')
  } catch (error) {
    // 【修复】catch 中的错误信息从 error 参数获取，而非 res
    // request 拦截器已经统一调用了 ElMessage.error，这里可以不重复提示
    // 但为了确保用户看到，再次打印（可根据需要去掉）
    const errMsg = error instanceof Error ? error.message : '登录失败，请稍后重试'
    console.error('登录失败:', errMsg)
    // 注意：request.ts 的响应拦截器已经调用了 ElMessage.error，
    // 这里不需要重复调用，避免弹出两条错误提示
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
/* 登录页面容器：全屏居中，渐变背景 */
.login-wrap {
  width: 100vw;
  height: 100vh;
  background: linear-gradient(135deg, #409eff, #66b1ff);
  display: flex;
  align-items: center;
  justify-content: center;
}

/* 登录卡片 */
.login-box {
  width: 420px;
  padding: 40px 30px;
  background: #fff;
  border-radius: 10px;
  box-shadow: 0 2px 16px rgba(0, 0, 0, 0.1);
}

.title {
  text-align: center;
  margin-bottom: 30px;
  color: #333;
}

/* 登录按钮：全宽 */
.login-btn {
  width: 100%;
  height: 42px;
  font-size: 16px;
}
</style>
