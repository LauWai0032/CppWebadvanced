<!--
  @file views/forum/ForumPostList.vue
  @brief 论坛帖子列表页面

  论坛帖子模块的主页面，提供增删改查（CRUD）功能。

  页面组成：
    1. 顶部搜索栏 + 新增按钮
    2. 中间数据表格
    3. 底部分页器
    4. 新增/编辑弹窗
    5. 删除确认

  @date 2025
-->

<template>
  <div class="forum_container">
    <!-- ==================== 顶部搜索栏 ==================== -->
    <el-card class="search-card" shadow="never">
      <el-form :inline="true" :model="searchForm" @submit.prevent="handleSearch">
        <el-form-item label="标题">
          <el-input
            v-model="searchForm.title"
            placeholder="请输入标题"
            clearable
            style="width: 180px"
          />
        </el-form-item>
        <el-form-item label="分类">
          <el-select
            v-model="searchForm.category"
            placeholder="全部分类"
            clearable
            style="width: 140px"
          >
            <el-option label="闲聊" value="chat" />
            <el-option label="失物招领" value="lost_found" />
            <el-option label="通知" value="notice" />
            <el-option label="其他" value="other" />
          </el-select>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :icon="Search" @click="handleSearch">搜索</el-button>
          <el-button :icon="Refresh" @click="handleReset">重置</el-button>
        </el-form-item>
      </el-form>
    </el-card>

    <!-- ==================== 数据表格区域 ==================== -->
    <el-card class="table-card" shadow="never">
      <div class="table-toolbar">
        <el-button type="primary" :icon="Plus" @click="handleAdd">新增</el-button>
      </div>

      <el-table
        v-loading="loading"
        :data="forumPostList"
        border
        stripe
        style="width: 100%"
      >
        <el-table-column type="index" label="序号" width="60" align="center" />
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="title" label="标题" width="200" />
        <el-table-column prop="category" label="分类" width="100" />
        <el-table-column prop="user_id" label="发帖人ID" width="100" />
        <el-table-column prop="status" label="状态" width="80" align="center">
          <template #default="{ row }">
            <el-tag :type="row.status === 1 ? 'success' : 'info'">
              {{ row.status === 1 ? '正常' : '禁用' }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="created_at" label="创建时间" width="180" />
        <el-table-column label="操作" width="220" fixed="right" align="center">
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
        label-width="100px"
      >
        <el-form-item label="标题" prop="title">
          <el-input v-model="formData.title" placeholder="请输入标题" />
        </el-form-item>
        <el-form-item label="发帖人ID" prop="user_id">
          <el-input-number v-model="formData.user_id" :min="0" style="width: 100%" />
        </el-form-item>
        <el-form-item label="分类" prop="category">
          <el-select v-model="formData.category" placeholder="请选择分类" style="width: 100%">
            <el-option label="闲聊" value="chat" />
            <el-option label="失物招领" value="lost_found" />
            <el-option label="通知" value="notice" />
            <el-option label="其他" value="other" />
          </el-select>
        </el-form-item>
        <el-form-item label="标签(逗号分隔)" prop="tags">
          <el-input v-model="formData.tags" placeholder="请输入标签(逗号分隔)" />
        </el-form-item>
        <el-form-item label="正文" prop="content">
          <el-input
            v-model="formData.content"
            type="textarea"
            :rows="4"
            placeholder="请输入正文"
          />
        </el-form-item>
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
 * @brief 论坛帖子列表页 - 脚本部分
 *
 * 使用 Vue 3 Composition API（<script setup> 语法糖）。
 */

import { ref, reactive, onMounted, computed } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { Search, Refresh, Plus, Edit, Delete } from '@element-plus/icons-vue'
import {
  getForumPostList, createForumPost, updateForumPost, deleteForumPost,
} from '@/api/forum'
import type { ForumPost, ForumPostForm, ForumPostQueryParams, PageResult } from '@/types'

// ==================== 搜索表单状态 ====================
const searchForm = reactive<ForumPostQueryParams>({
  page: 1,
  pageSize: 10,
  title: "",
  category: "",
})

// ==================== 表格数据与分页状态 ====================
const loading = ref(false)
const forumPostList = ref<ForumPost[]>([])

const pagination = reactive({
  page: 1,
  pageSize: 10,
  total: 0,
})

// ==================== 弹窗表单状态 ====================
const dialogVisible = ref(false)
const formRef = ref<FormInstance>()
const formData = reactive<ForumPostForm>({
  title: "",
  user_id: 0,
  category: "chat",
  tags: "",
  content: "",
  status: 1,
})

const isEdit = ref(false)
const submitLoading = ref(false)

const dialogTitle = computed(() => (isEdit.value ? '编辑论坛帖子' : '新增论坛帖子'))

const formRules: FormRules = {
  title: [
    { required: true, message: '请输入标题', trigger: 'blur' },
  ],
  user_id: [
    { required: true, message: '请输入发帖人ID', trigger: 'blur' },
  ],
  category: [
    { required: true, message: '请输入分类', trigger: 'change' },
  ],
  content: [
    { required: true, message: '请输入正文', trigger: 'blur' },
  ],
  status: [
    { required: true, message: '请输入状态', trigger: 'change' },
  ],
}

// ==================== 数据加载函数 ====================
async function loadList() {
  loading.value = true
  try {
    const params: ForumPostQueryParams = {
      page: pagination.page,
      pageSize: pagination.pageSize,
    }

    if (searchForm.title) {
      params.title = searchForm.title
    }
    if (searchForm.category) {
      params.category = searchForm.category
    }

    const data = await getForumPostList(params)
    forumPostList.value = data.list
    pagination.total = data.total
  } catch (error) {
    console.error('加载列表失败:', error)
  } finally {
    loading.value = false
  }
}

// ==================== 搜索操作 ====================
function handleSearch() {
  pagination.page = 1
  loadList()
}

function handleReset() {
  searchForm.title = ''
  searchForm.category = ''
  pagination.page = 1
  loadList()
}

// ==================== 新增/编辑操作 ====================
function handleAdd() {
  isEdit.value = false
  Object.assign(formData, {
    title: "",
    user_id: 0,
    category: "chat",
    tags: "",
    content: "",
    status: 1,
    id: undefined,
  })
  formRef.value?.clearValidate()
  dialogVisible.value = true
}

function handleEdit(row: ForumPost) {
  isEdit.value = true
  Object.assign(formData, {
    id: row.id,
    title: row.title,
    user_id: row.user_id,
    category: row.category,
    tags: row.tags,
    content: row.content,
    status: row.status,
  })
  formRef.value?.clearValidate()
  dialogVisible.value = true
}

async function handleSubmit() {
  if (!formRef.value) return
  try {
    await formRef.value.validate()
  } catch {
    return
  }

  submitLoading.value = true
  try {
    if (isEdit.value && formData.id) {
      await updateForumPost(formData.id, formData)
      ElMessage.success('更新成功')
    } else {
      await createForumPost(formData)
      ElMessage.success('创建成功')
    }
    dialogVisible.value = false
    loadList()
  } catch (error) {
    console.error('提交失败:', error)
  } finally {
    submitLoading.value = false
  }
}

// ==================== 删除操作 ====================
function handleDelete(row: ForumPost) {
  ElMessageBox.confirm(
    `确定要删除吗？删除后不可恢复！`
    '删除确认',
    {
      confirmButtonText: '确定删除',
      cancelButtonText: '取消',
      type: 'warning',
    },
  )
    .then(async () => {
      try {
        await deleteForumPost(row.id)
        ElMessage.success('删除成功')
        if (forumPostList.value.length === 1 && pagination.page > 1) {
          pagination.page -= 1
        }
        loadList()
      } catch (error) {
        console.error('删除失败:', error)
      }
    })
    .catch(() => {})
}

// ==================== 分页操作 ====================
function handleSizeChange(size: number) {
  pagination.pageSize = size
  pagination.page = 1
  loadList()
}

function handlePageChange(page: number) {
  pagination.page = page
  loadList()
}

// ==================== 页面初始化 ====================
onMounted(() => {
  loadList()
})
</script>

<style scoped>
.forum_container {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.search-card {
  margin-bottom: 0;
}

.table-card {
  flex: 1;
}

.table-toolbar {
  margin-bottom: 16px;
}

.pagination-wrapper {
  display: flex;
  justify-content: flex-end;
  margin-top: 20px;
}
</style>
