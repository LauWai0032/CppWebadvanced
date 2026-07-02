import { fileURLToPath, URL } from 'node:url'
import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import vueDevTools from 'vite-plugin-vue-devtools'

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    vueDevTools(),
  ],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    },
  },
  
  // 👇 在这里添加服务器配置
  server: {
    proxy: {
      // 拦截所有以 /api 开头的请求
      '/api': {
        target: 'http://localhost:8080', // 你的 C++ 后端地址
        changeOrigin: true,               // 支持跨域
        // rewrite: (path) => path.replace(/^\/api/, '') // 暂时不重写路径
      }
    }
  }
})