<template>
  <div class="app-container">
    <h1>Vue3 + C++ 后端联调测试</h1>

    <!-- 1. 发送 GET 请求示例 -->
    <section>
      <h3>获取数据 (GET)</h3>
      <button @click="fetchData" :disabled="loading">获取欢迎信息</button>
      <p v-if="loading">加载中...</p>
      <p v-if="errorMsg" style="color: red;">{{ errorMsg }}</p>
      <p v-if="getData">{{ getData }}</p>
    </section>

    <hr />

    <!-- 2. 发送 POST 请求示例 -->
    <section>
      <h3>提交数据 (POST)</h3>
      <input v-model="username" placeholder="请输入用户名" />
      <input v-model="password" type="password" placeholder="请输入密码" />
      <button @click="postData" :disabled="loading">登录</button>
      
      <p v-if="loading">提交中...</p>
      <p v-if="errorMsg" style="color: red;">{{ errorMsg }}</p>
      <p v-if="postDataResult">后端返回结果：{{ JSON.stringify(postDataResult) }}</p>
    </section>
  </div>
</template>

<script setup>
import { ref } from 'vue';
import axios from 'axios';

// 定义响应式状态
const loading = ref(false);
const errorMsg = ref('');
const getData = ref('');
const postDataResult = ref(null);
const username = ref('');
const password = ref('');

// 1. 处理 GET 请求
const fetchData = async () => {
  loading.value = true;
  errorMsg.value = '';
  try {
    // 请求你 C++ 后端提供的 GET 接口
    const response = await axios.get('/api/hello'); 
    getData.value = response.data.msg;
  } catch (error) {
    errorMsg.value = '获取数据失败: ' + (error.response?.data?.msg || error.message);
  } finally {
    loading.value = false;
  }
};

// 2. 处理 POST 请求
const postData = async () => {
  loading.value = true;
  errorMsg.value = '';
  try {
    // 请求你 C++ 后端提供的 POST 接口，并传递 JSON 数据
    const response = await axios.post('/api/login', {
      username: username.value,
      password: password.value
    });
    postDataResult.value = response.data;
  } catch (error) {
    errorMsg.value = '登录失败: ' + (error.response?.data?.msg || error.message);
  } finally {
    loading.value = false;
  }
};
</script>

<style scoped>
.app-container {
  max-width: 600px;
  margin: 40px auto;
  padding: 20px;
  font-family: sans-serif;
}
section {
  margin-bottom: 20px;
}
button {
  padding: 8px 16px;
  margin-right: 10px;
  cursor: pointer;
}
input {
  padding: 8px;
  margin-right: 10px;
  margin-bottom: 10px;
}
</style>