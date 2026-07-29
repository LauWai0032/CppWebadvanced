<template>
  <div class="login-wrap">
    <div class="login-box">
      <h2 class="title">社区物业管理平台</h2>
      <el-form ref="loginFormRef" :model="loginForm" label-width="80px">
        <el-form-item label="账号" prop="username">
          <el-input v-model="loginForm.username" placeholder="请输入账号"></el-input>
        </el-form-item>
        <el-form-item label="密码" prop="password">
          <el-input v-model="loginForm.password" show-password placeholder="请输入密码"></el-input>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" class="login-btn" @click="handleLogin">登录</el-button>
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage } from 'element-plus'
import { loginApi } from '@/api/login'

const router = useRouter()
const loginFormRef = ref(null)

// 登录表单
const loginForm = ref({
  username: '',
  password: ''
})

// 登录逻辑
const handleLogin = async () => {
  if (!loginForm.value.username || !loginForm.value.password) {
    ElMessage.warning('账号密码不能为空')
    return
  }
  try {
    const res = await loginApi(loginForm.value)
    // 后端返回格式示例：{code:200, msg:"success", data:{token:"xxx", role:"owner"}}
    if (res.data.code === 200) {
      // 存储登录凭证
      localStorage.setItem('token', res.data.data.token)
      localStorage.setItem('userInfo', JSON.stringify(res.data.data))
      ElMessage.success('登录成功')
      router.push('/home')
    } else {
      ElMessage.error(res.data.msg || '账号或密码错误')
    }
  } catch (err) {
    ElMessage.error('服务器连接失败，请检查后端服务')
  }
}
</script>

<style scoped>
.login-wrap {
  width: 100vw;
  height: 100vh;
  background: linear-gradient(135deg, #409eff, #66b1ff);
  display: flex;
  align-items: center;
  justify-content: center;
}
.login-box {
  width: 420px;
  padding: 40px 30px;
  background: #fff;
  border-radius: 10px;
  box-shadow: 0 2px 16px rgba(0,0,0,0.1);
}
.title {
  text-align: center;
  margin-bottom: 30px;
  color: #333;
}
.login-btn {
  width: 100%;
  height: 42px;
  font-size: 16px;
}
</style>
