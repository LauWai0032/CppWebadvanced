<!--
  @file views/user/UserList.vue
  @brief 用户列表页面

  用户管理模块的主页面，提供用户的增删改查（CRUD）功能。
  作为样板模块供后续 7 个业务模块参考复制。

  页面组成：
    1. 顶部搜索栏：用户名/角色筛选 + 新增按钮
    2. 中间数据表格：展示用户列表（Element Plus Table）
    3. 底部分页器：Element Plus Pagination
    4. 新增/编辑弹窗：Element Plus Dialog + Form
    5. 删除确认：Element Plus MessageBox 确认弹窗

  与后端 API 的对接：
    - 加载列表：  GET    /api/users    (getUserList)
    - 新增用户：  POST   /api/users    (createUser)
    - 更新用户：  PUT    /api/users/id (updateUser)
    - 删除用户：  DELETE /api/users/id (deleteUser)

  设计模式：
    - 数据驱动视图：所有表格/分页/弹窗状态均由 data/ref 驱动
    - 复用原则：搜索、分页、弹窗等模式可直接复用到其他模块
-->

<template>
  <div class="user-list-container">
    <!-- ==================== 顶部搜索栏 ==================== -->
    <el-card class="search-card" shadow="never">
      <el-form :inline="true" :model="searchForm" @submit.prevent="handleSearch">
        <!-- 用户名搜索 -->
        <el-form-item label="用户名">
          <el-input
            v-model="searchForm.username"
            placeholder="请输入用户名"
            clearable
            style="width: 200px"
          />
        </el-form-item>

        <!-- 角色筛选 -->
        <el-form-item label="角色">
          <el-select
            v-model="searchForm.role"
            placeholder="全部角色"
            clearable
            style="width: 150px"
          >
            <el-option label="管理员" value="admin" />
            <el-option label="物业" value="property" />
            <el-option label="业主" value="owner" />
          </el-select>
        </el-form-item>

        <!-- 状态筛选 -->
        <el-form-item label="状态">
          <el-select
            v-model="searchForm.status"
            placeholder="全部状态"
            clearable
            style="width: 120px"
          >
            <el-option label="正常" :value="1" />
            <el-option label="禁用" :value="0" />
          </el-select>
        </el-form-item>

        <!-- 搜索按钮 -->
        <el-form-item>
          <el-button type="primary" :icon="Search" @click="handleSearch">搜索</el-button>
          <el-button :icon="Refresh" @click="handleReset">重置</el-button>
        </el-form-item>
      </el-form>
    </el-card>

    <!-- ==================== 数据表格区域 ==================== -->
    <el-card class="table-card" shadow="never">
      <!-- 表格工具栏：新增按钮 -->
      <div class="table-toolbar">
        <el-button type="primary" :icon="Plus" @click="handleAdd">新增用户</el-button>
      </div>

      <!-- 用户列表表格 -->
      <!-- v-loading 绑定加载状态，请求期间显示 loading 遮罩 -->
      <el-table
        v-loading="loading"
        :data="userList"
        border
        stripe
        style="width: 100%"
      >
        <!-- 序号列（固定左侧） -->
        <el-table-column type="index" label="序号" width="60" align="center" />

        <!-- 用户 ID -->
        <el-table-column prop="id" label="ID" width="80" align="center" />

        <!-- 用户名 -->
        <el-table-column prop="username" label="用户名" min-width="120" />

        <!-- 真实姓名 -->
        <el-table-column prop="real_name" label="真实姓名" min-width="100" />

        <!-- 手机号 -->
        <el-table-column prop="phone" label="手机号" width="130" />

        <!-- 角色（带标签样式） -->
        <el-table-column prop="role" label="角色" width="100" align="center">
          <template #default="{ row }">
            <el-tag :type="roleTagType(row.role)">
              {{ roleLabel(row.role) }}
            </el-tag>
          </template>
        </el-table-column>

        <!-- 状态（开关样式） -->
        <el-table-column prop="status" label="状态" width="80" align="center">
          <template #default="{ row }">
            <el-tag :type="row.status === 1 ? 'success' : 'info'">
              {{ row.status === 1 ? '正常' : '禁用' }}
            </el-tag>
          </template>
        </el-table-column>

        <!-- 创建时间 -->
        <el-table-column prop="created_at" label="创建时间" width="180" />

        <!-- 操作列（固定右侧） -->
        <el-table-column label="操作" width="180" fixed="right" align="center">
          <template #default="{ row }">
            <el-button type="primary" link :icon="Edit" @click="handleEdit(row)">编辑</el-button>
            <el-button type="danger" link :icon="Delete" @click="handleDelete(row)">删除</el-button>
          </template>
        </el-table-column>
      </el-table>

      <!-- ==================== 分页器 ==================== -->
      <div class="pagination-wrapper">
        <el-pagination
          v-model:current-page="pagination.page"
          v-model:page-size="pagination.pageSize"
          :page-sizes="[10, 20, 50, 100]"
          :total="pagination.total"
          layout="total, sizes, prev, pager, next, jumper"
          background
          @size-change="handleSizeChange"
          @current-change="handlePageChange"
        />
      </div>
    </el-card>

    <!-- ==================== 新增/编辑弹窗 ==================== -->
    <el-dialog
      v-model="dialogVisible"
      :title="dialogTitle"
      width="500px"
      :close-on-click-modal="false"
    >
      <el-form
        ref="formRef"
        :model="formData"
        :rules="formRules"
        label-width="80px"
      >
        <!-- 用户名 -->
        <el-form-item label="用户名" prop="username">
          <el-input v-model="formData.username" placeholder="请输入用户名" />
        </el-form-item>

        <!-- 密码（新增时必填，编辑时可空表示不修改） -->
        <el-form-item label="密码" prop="password">
          <el-input
            v-model="formData.password"
            type="password"
            show-password
            :placeholder="isEdit ? '不修改请留空' : '请输入密码'"
          />
        </el-form-item>

        <!-- 真实姓名 -->
        <el-form-item label="真实姓名" prop="real_name">
          <el-input v-model="formData.real_name" placeholder="请输入真实姓名" />
        </el-form-item>

        <!-- 手机号 -->
        <el-form-item label="手机号" prop="phone">
          <el-input v-model="formData.phone" placeholder="请输入手机号" />
        </el-form-item>

        <!-- 角色 -->
        <el-form-item label="角色" prop="role">
          <el-select v-model="formData.role" placeholder="请选择角色" style="width: 100%">
            <el-option label="管理员" value="admin" />
            <el-option label="物业" value="property" />
            <el-option label="业主" value="owner" />
          </el-select>
        </el-form-item>

        <!-- 状态 -->
        <el-form-item label="状态" prop="status">
          <el-radio-group v-model="formData.status">
            <el-radio :value="1">正常</el-radio>
            <el-radio :value="0">禁用</el-radio>
          </el-radio-group>
        </el-form-item>
      </el-form>

      <template #footer>
        <el-button @click="dialogVisible = false">取消</el-button>
        <el-button type="primary" :loading="submitLoading" @click="handleSubmit">
          确定
        </el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
/**
 * @brief 用户列表页 - 脚本部分
 *
 * 使用 Vue 3 Composition API（<script setup> 语法糖）。
 * 所有逻辑按功能区块组织，便于阅读和维护。
 *
 * 主要功能模块：
 *   1. 导入依赖
 *   2. 搜索表单状态
 *   3. 表格数据与分页状态
 *   4. 弹窗表单状态与校验规则
 *   5. 辅助函数（角色标签、状态映射等）
 *   6. 数据加载函数
 *   7. 搜索/重置操作
 *   8. 新增/编辑操作
 *   9. 删除操作
 *   10. 分页操作
 *   11. 页面初始化
 */

import { ref, reactive, onMounted, computed } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { Search, Refresh, Plus, Edit, Delete } from '@element-plus/icons-vue'
import {
  getUserList,
  createUser,
  updateUser,
  deleteUser,
} from '@/api/user'
import type { User, UserForm, UserQueryParams, UserRole, PageResult } from '@/types'

// ==================== 搜索表单状态 ====================
/**
 * @brief 搜索表单数据
 *
 * 与顶部搜索栏的输入项双向绑定。
 * 点击"搜索"按钮时将此对象作为查询参数传给后端。
 */
const searchForm = reactive<UserQueryParams>({
  page: 1,
  pageSize: 10,
  username: '',
  role: '',
  status: undefined,
})

// ==================== 表格数据与分页状态 ====================

/** 表格加载状态（控制 v-loading 显示） */
const loading = ref(false)

/** 用户列表数据（表格数据源） */
const userList = ref<User[]>([])

/**
 * @brief 分页信息
 *
 * - page:     当前页码（从 1 开始）
 * - pageSize: 每页条数
 * - total:    总记录数（从后端返回）
 */
const pagination = reactive({
  page: 1,
  pageSize: 10,
  total: 0,
})

// ==================== 弹窗表单状态 ====================

/** 弹窗是否可见 */
const dialogVisible = ref(false)

/** 表单引用（用于校验和重置） */
const formRef = ref<FormInstance>()

/** 表单数据（新增/编辑共用） */
const formData = reactive<UserForm>({
  username: '',
  password: '',
  real_name: '',
  phone: '',
  role: 'owner',
  status: 1,
})

/** 是否为编辑模式（true=编辑 false=新增） */
const isEdit = ref(false)

/** 表单提交加载状态（防止重复提交） */
const submitLoading = ref(false)

/**
 * @brief 弹窗标题
 *
 * 根据 isEdit 动态计算标题文字。
 * 使用 computed 确保响应式更新。
 */
const dialogTitle = computed(() => (isEdit.value ? '编辑用户' : '新增用户'))

/**
 * @brief 表单校验规则
 *
 * 使用 Element Plus Form 的 rules 配置。
 * 编辑模式下密码不是必填（留空表示不修改）。
 *
 * 注意：密码的必填/选填逻辑在 rules 中统一处理，
 * 但实际切换模式时需要动态调整，此处简化为统一可选，
 * 提交时再判断（新增时不能为空）。
 */
const formRules: FormRules = {
  username: [
    { required: true, message: '请输入用户名', trigger: 'blur' },
    { min: 3, max: 20, message: '用户名长度为 3-20 个字符', trigger: 'blur' },
  ],
  real_name: [
    { max: 20, message: '真实姓名不能超过 20 个字符', trigger: 'blur' },
  ],
  phone: [
    {
      pattern: /^1[3-9]\d{9}$/,
      message: '请输入正确的手机号',
      trigger: 'blur',
    },
  ],
  role: [
    { required: true, message: '请选择角色', trigger: 'change' },
  ],
  status: [
    { required: true, message: '请选择状态', trigger: 'change' },
  ],
}

// ==================== 辅助函数 ====================

/**
 * @brief 获取角色对应的中文标签
 * @param role 角色标识
 * @returns 中文角色名
 */
function roleLabel(role: UserRole | string): string {
  const map: Record<string, string> = {
    admin: '管理员',
    property: '物业',
    owner: '业主',
  }
  return map[role] || role
}

/**
 * @brief 获取角色标签的颜色类型
 * @param role 角色标识
 * @returns Element Plus Tag 组件的 type 值
 */
function roleTagType(role: UserRole | string): 'danger' | 'warning' | 'primary' {
  const map: Record<string, 'danger' | 'warning' | 'primary'> = {
    admin: 'danger',
    property: 'warning',
    owner: 'primary',
  }
  return map[role] || 'primary'
}

// ==================== 数据加载函数 ====================

/**
 * @brief 加载用户列表数据
 *
 * 调用后端分页查询接口，获取当前页的用户数据。
 * 自动处理 loading 状态和异常情况。
 *
 * 调用时机：
 *   - 页面初次加载（onMounted）
 *   - 搜索条件改变（点击搜索按钮）
 *   - 页码变化（切换页面）
 *   - 每页条数变化
 *   - 新增/编辑/删除操作后刷新
 */
async function loadUserList() {
  loading.value = true
  try {
    // 组装查询参数：合并搜索条件和分页参数
    const params: UserQueryParams = {
      page: pagination.page,
      pageSize: pagination.pageSize,
    }

    // 搜索条件非空时才传参，避免后端收到空字符串影响查询
    if (searchForm.username.trim()) {
      params.username = searchForm.username.trim()
    }
    if (searchForm.role) {
      params.role = searchForm.role
    }
    if (searchForm.status !== undefined && searchForm.status !== null) {
      params.status = searchForm.status
    }

    // 调用 API（request 拦截器已自动剥离外层包装，直接拿到 PageResult）
    const data = await getUserList(params)

    // 更新表格数据
    userList.value = data.list
    pagination.total = data.total
  } catch (error) {
    // 错误提示已由 request 拦截器统一处理（ElMessage.error）
    // 此处仅打印调试信息
    console.error('加载用户列表失败:', error)
  } finally {
    // 无论成功失败，都关闭 loading
    loading.value = false
  }
}

// ==================== 搜索操作 ====================

/**
 * @brief 点击搜索按钮
 *
 * 重置到第 1 页并重新加载数据。
 * 搜索条件变化后应该从第 1 页开始展示，
 * 否则可能出现"当前页超过总页数"的情况。
 */
function handleSearch() {
  pagination.page = 1
  loadUserList()
}

/**
 * @brief 点击重置按钮
 *
 * 清空所有搜索条件，重置为默认值，然后重新加载。
 */
function handleReset() {
  searchForm.username = ''
  searchForm.role = ''
  searchForm.status = undefined
  pagination.page = 1
  loadUserList()
}

// ==================== 新增/编辑操作 ====================

/**
 * @brief 点击新增按钮
 *
 * 重置表单数据，设置为新增模式，打开弹窗。
 */
function handleAdd() {
  isEdit.value = false
  // 重置表单数据为默认值
  Object.assign(formData, {
    username: '',
    password: '',
    real_name: '',
    phone: '',
    role: 'owner' as UserRole,
    status: 1,
    id: undefined,
  })
  // 清除表单校验状态
  formRef.value?.clearValidate()
  dialogVisible.value = true
}

/**
 * @brief 点击编辑按钮
 *
 * 填充当前行数据到表单，设置为编辑模式，打开弹窗。
 * 注意：编辑模式下密码字段留空（不显示原密码，也不修改）
 *
 * @param row 当前行的用户数据
 */
function handleEdit(row: User) {
  isEdit.value = true
  // 填充表单数据（从表格行数据复制）
  Object.assign(formData, {
    id: row.id,
    username: row.username,
    password: '', // 编辑时密码留空，表示不修改
    real_name: row.real_name,
    phone: row.phone,
    role: row.role,
    status: row.status,
  })
  // 清除之前的校验状态
  formRef.value?.clearValidate()
  dialogVisible.value = true
}

/**
 * @brief 提交表单（新增或更新）
 *
 * 根据 isEdit 判断调用创建还是更新接口。
 * 提交前先做表单校验，校验通过才发请求。
 * 成功后关闭弹窗并刷新列表。
 */
async function handleSubmit() {
  // 表单校验
  if (!formRef.value) return
  try {
    await formRef.value.validate()
  } catch {
    // 校验不通过，直接返回
    return
  }

  // 新增时密码必填
  if (!isEdit.value && !formData.password.trim()) {
    ElMessage.warning('请设置用户密码')
    return
  }

  submitLoading.value = true
  try {
    if (isEdit.value && formData.id) {
      // 编辑模式：调用更新接口
      await updateUser(formData.id, formData)
      ElMessage.success('更新成功')
    } else {
      // 新增模式：调用创建接口
      await createUser(formData)
      ElMessage.success('创建成功')
    }

    // 关闭弹窗，刷新列表
    dialogVisible.value = false
    loadUserList()
  } catch (error) {
    // 错误已由拦截器提示
    console.error('提交失败:', error)
  } finally {
    submitLoading.value = false
  }
}

// ==================== 删除操作 ====================

/**
 * @brief 点击删除按钮
 *
 * 弹出确认对话框，用户确认后调用删除接口。
 * 删除成功后刷新当前页数据。
 *
 * @param row 当前行的用户数据
 */
function handleDelete(row: User) {
  ElMessageBox.confirm(
    `确定要删除用户「${row.username}」吗？删除后不可恢复！`,
    '删除确认',
    {
      confirmButtonText: '确定删除',
      cancelButtonText: '取消',
      type: 'warning',
    },
  )
    .then(async () => {
      try {
        await deleteUser(row.id)
        ElMessage.success('删除成功')
        // 删除后重新加载列表
        // 如果当前页只剩最后一条数据，删除后应回到上一页
        if (userList.value.length === 1 && pagination.page > 1) {
          pagination.page -= 1
        }
        loadUserList()
      } catch (error) {
        console.error('删除失败:', error)
      }
    })
    .catch(() => {
      // 用户点击取消，什么也不做
    })
}

// ==================== 分页操作 ====================

/**
 * @brief 切换每页条数
 * @param size 新的每页条数
 */
function handleSizeChange(size: number) {
  pagination.pageSize = size
  pagination.page = 1 // 切换页大小时回到第 1 页
  loadUserList()
}

/**
 * @brief 切换页码
 * @param page 新的页码
 */
function handlePageChange(page: number) {
  pagination.page = page
  loadUserList()
}

// ==================== 页面初始化 ====================
/**
 * 组件挂载完成后加载第一页数据。
 */
onMounted(() => {
  loadUserList()
})
</script>

<style scoped>
/* 页面容器 */
.user-list-container {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

/* 搜索卡片 */
.search-card {
  margin-bottom: 0;
}

/* 表格卡片 */
.table-card {
  /* 利用 flex 布局占满剩余空间 */
  flex: 1;
}

/* 表格工具栏 */
.table-toolbar {
  margin-bottom: 16px;
}

/* 分页器容器（右对齐） */
.pagination-wrapper {
  display: flex;
  justify-content: flex-end;
  margin-top: 20px;
}
</style>
