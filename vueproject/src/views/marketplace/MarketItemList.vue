<!--
  @file views/marketplace/MarketItemList.vue
  @brief 二手交易列表页面

  二手交易模块的主页面，提供增删改查（CRUD）功能。

  页面组成：
    1. 顶部搜索栏 + 新增按钮
    2. 中间数据表格
    3. 底部分页器
    4. 新增/编辑弹窗
    5. 删除确认

  @date 2025
-->

<template>
  <div class="marketplace_container">
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
        <el-form-item label="交易方式">
          <el-select
            v-model="searchForm.exchange_type"
            placeholder="全部交易方式"
            clearable
            style="width: 140px"
          >
            <el-option label="出售" value="sell" />
            <el-option label="以物易物" value="barter" />
          </el-select>
        </el-form-item>
        <el-form-item label="状态">
          <el-select
            v-model="searchForm.status"
            placeholder="全部状态"
            clearable
            style="width: 140px"
          >
            <el-option label="在售" value="active" />
            <el-option label="已售出" value="sold" />
            <el-option label="已关闭" value="closed" />
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
        :data="marketItemList"
        border
        stripe
        style="width: 100%"
      >
        <el-table-column type="index" label="序号" width="60" align="center" />
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="title" label="标题" width="180" />
        <el-table-column prop="exchange_type" label="交易方式" width="100" />
        <el-table-column label="价格/期望物品" width="180">
          <template #default="{ row }">
            <div v-if="row.exchange_type === 'sell'">¥{{ row.price }}</div>
            <div v-else>{{ row.expect_item || '-' }}</div>
          </template>
        </el-table-column>
        <el-table-column prop="status" label="状态" width="100" align="center">
          <template #default="{ row }">
            <el-tag>{{ row.status }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="seller_id" label="发布者ID" width="100" />
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
        <el-form-item label="物品标题" prop="title">
          <el-input v-model="formData.title" placeholder="请输入物品标题" />
        </el-form-item>
        <el-form-item label="发布者ID" prop="seller_id">
          <el-input-number v-model="formData.seller_id" :min="0" style="width: 100%" />
        </el-form-item>
        <el-form-item label="交易方式" prop="exchange_type">
          <el-select v-model="formData.exchange_type" placeholder="请选择交易方式" style="width: 100%">
            <el-option label="出售" value="sell" />
            <el-option label="以物易物" value="barter" />
          </el-select>
        </el-form-item>
        <el-form-item label="售价" prop="price">
          <el-input-number v-model="formData.price" :min="0" style="width: 100%" />
        </el-form-item>
        <el-form-item label="期望交换物品" prop="expect_item">
          <el-input v-model="formData.expect_item" placeholder="请输入期望交换物品" />
        </el-form-item>
        <el-form-item label="物品描述" prop="description">
          <el-input
            v-model="formData.description"
            type="textarea"
            :rows="4"
            placeholder="请输入物品描述"
          />
        </el-form-item>
        <el-form-item label="状态" prop="status">
          <el-select v-model="formData.status" placeholder="请选择状态" style="width: 100%">
            <el-option label="在售" value="active" />
            <el-option label="已售出" value="sold" />
            <el-option label="已关闭" value="closed" />
          </el-select>
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
 * @brief 二手交易列表页 - 脚本部分
 *
 * 使用 Vue 3 Composition API（<script setup> 语法糖）。
 */

import { ref, reactive, onMounted, computed } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { Search, Refresh, Plus, Edit, Delete } from '@element-plus/icons-vue'
import {
  getMarketplaceItemList, createMarketplaceItem, updateMarketplaceItem, deleteMarketplaceItem,
} from '@/api/marketplace'
import type { MarketplaceItem, MarketplaceItemForm, MarketplaceItemQueryParams, PageResult } from '@/types'

// ==================== 搜索表单状态 ====================
const searchForm = reactive<MarketplaceItemQueryParams>({
  page: 1,
  pageSize: 10,
  title: "",
  exchange_type: "",
  status: "",
})

// ==================== 表格数据与分页状态 ====================
const loading = ref(false)
const marketItemList = ref<MarketplaceItem[]>([])

const pagination = reactive({
  page: 1,
  pageSize: 10,
  total: 0,
})

// ==================== 弹窗表单状态 ====================
const dialogVisible = ref(false)
const formRef = ref<FormInstance>()
const formData = reactive<MarketplaceItemForm>({
  title: "",
  seller_id: 0,
  exchange_type: "sell",
  price: 0,
  expect_item: "",
  description: "",
  status: "active",
})

const isEdit = ref(false)
const submitLoading = ref(false)

const dialogTitle = computed(() => (isEdit.value ? '编辑二手交易' : '新增二手交易'))

const formRules: FormRules = {
  title: [
    { required: true, message: '请输入物品标题', trigger: 'blur' },
  ],
  seller_id: [
    { required: true, message: '请输入发布者ID', trigger: 'blur' },
  ],
  exchange_type: [
    { required: true, message: '请输入交易方式', trigger: 'change' },
  ],
  status: [
    { required: true, message: '请输入状态', trigger: 'change' },
  ],
}

// ==================== 数据加载函数 ====================
async function loadList() {
  loading.value = true
  try {
    const params: MarketplaceItemQueryParams = {
      page: pagination.page,
      pageSize: pagination.pageSize,
    }

    if (searchForm.title) {
      params.title = searchForm.title
    }
    if (searchForm.exchange_type) {
      params.exchange_type = searchForm.exchange_type
    }
    if (searchForm.status) {
      params.status = searchForm.status
    }

    const data = await getMarketplaceItemList(params)
    marketItemList.value = data.list
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
  searchForm.exchange_type = ''
  searchForm.status = ''
  pagination.page = 1
  loadList()
}

// ==================== 新增/编辑操作 ====================
function handleAdd() {
  isEdit.value = false
  Object.assign(formData, {
    title: "",
    seller_id: 0,
    exchange_type: "sell",
    price: 0,
    expect_item: "",
    description: "",
    status: "active",
    id: undefined,
  })
  formRef.value?.clearValidate()
  dialogVisible.value = true
}

function handleEdit(row: MarketplaceItem) {
  isEdit.value = true
  Object.assign(formData, {
    id: row.id,
    title: row.title,
    seller_id: row.seller_id,
    exchange_type: row.exchange_type,
    price: row.price,
    expect_item: row.expect_item,
    description: row.description,
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
      await updateMarketplaceItem(formData.id, formData)
      ElMessage.success('更新成功')
    } else {
      await createMarketplaceItem(formData)
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
function handleDelete(row: MarketplaceItem) {
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
        await deleteMarketplaceItem(row.id)
        ElMessage.success('删除成功')
        if (marketItemList.value.length === 1 && pagination.page > 1) {
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
.marketplace_container {
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
