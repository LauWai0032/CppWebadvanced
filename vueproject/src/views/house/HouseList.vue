<!--
  @file views/house/HouseList.vue
  @brief 房屋管理列表页面

  房屋管理模块的主页面，提供增删改查（CRUD）功能。

  页面组成：
    1. 顶部搜索栏 + 新增按钮
    2. 中间数据表格
    3. 底部分页器
    4. 新增/编辑弹窗
    5. 删除确认

  @date 2025
-->

<template>
  <div class="house_container">
    <!-- ==================== 顶部搜索栏 ==================== -->
    <el-card class="search-card" shadow="never">
      <el-form :inline="true" :model="searchForm" @submit.prevent="handleSearch">
        <el-form-item label="楼栋号">
          <el-input
            v-model="searchForm.building_no"
            placeholder="请输入楼栋号"
            clearable
            style="width: 180px"
          />
        </el-form-item>
        <el-form-item label="门牌号">
          <el-input
            v-model="searchForm.room_no"
            placeholder="请输入门牌号"
            clearable
            style="width: 180px"
          />
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
        <el-button type="primary" :icon="Plus" @click="handleAdd">新增房屋</el-button>
      </div>

      <el-table
        v-loading="loading"
        :data="houseList"
        border
        stripe
        style="width: 100%"
      >
        <el-table-column type="index" label="序号" width="60" align="center" />
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="building_no" label="楼栋号" width="100" />
        <el-table-column prop="unit_no" label="单元号" width="100" />
        <el-table-column prop="room_no" label="门牌号" width="100" />
        <el-table-column prop="area" label="面积" width="100" />
        <el-table-column prop="owner_id" label="业主ID" width="100" />
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
        <el-form-item label="楼栋号" prop="building_no">
          <el-input v-model="formData.building_no" placeholder="请输入楼栋号" />
        </el-form-item>
        <el-form-item label="单元号" prop="unit_no">
          <el-input v-model="formData.unit_no" placeholder="请输入单元号" />
        </el-form-item>
        <el-form-item label="门牌号" prop="room_no">
          <el-input v-model="formData.room_no" placeholder="请输入门牌号" />
        </el-form-item>
        <el-form-item label="面积(平米)" prop="area">
          <el-input-number v-model="formData.area" :min="0" style="width: 100%" />
        </el-form-item>
        <el-form-item label="业主ID" prop="owner_id">
          <el-input-number v-model="formData.owner_id" :min="0" style="width: 100%" />
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
 * @brief 房屋管理列表页 - 脚本部分
 *
 * 使用 Vue 3 Composition API（<script setup> 语法糖）。
 */

import { ref, reactive, onMounted, computed } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { Search, Refresh, Plus, Edit, Delete } from '@element-plus/icons-vue'
import {
  getHouseList, createHouse, updateHouse, deleteHouse,
} from '@/api/house'
import type { House, HouseForm, HouseQueryParams, PageResult } from '@/types'

// ==================== 搜索表单状态 ====================
const searchForm = reactive<HouseQueryParams>({
  page: 1,
  pageSize: 10,
  building_no: "",
  room_no: "",
})

// ==================== 表格数据与分页状态 ====================
const loading = ref(false)
const houseList = ref<House[]>([])

const pagination = reactive({
  page: 1,
  pageSize: 10,
  total: 0,
})

// ==================== 弹窗表单状态 ====================
const dialogVisible = ref(false)
const formRef = ref<FormInstance>()
const formData = reactive<HouseForm>({
  building_no: "",
  unit_no: "",
  room_no: "",
  area: 0,
  owner_id: 0,
})

const isEdit = ref(false)
const submitLoading = ref(false)

const dialogTitle = computed(() => (isEdit.value ? '编辑房屋' : '新增房屋'))

const formRules: FormRules = {
  building_no: [
    { required: true, message: '请输入楼栋号', trigger: 'blur' },
  ],
  room_no: [
    { required: true, message: '请输入门牌号', trigger: 'blur' },
  ],
  area: [
    { required: true, message: '请输入面积(平米)', trigger: 'blur' },
  ],
  owner_id: [
    { required: true, message: '请输入业主ID', trigger: 'blur' },
  ],
}

// ==================== 数据加载函数 ====================
async function loadList() {
  loading.value = true
  try {
    const params: HouseQueryParams = {
      page: pagination.page,
      pageSize: pagination.pageSize,
    }

    if (searchForm.building_no) {
      params.building_no = searchForm.building_no
    }
    if (searchForm.room_no) {
      params.room_no = searchForm.room_no
    }

    const data = await getHouseList(params)
    houseList.value = data.list
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
  searchForm.building_no = ''
  searchForm.room_no = ''
  pagination.page = 1
  loadList()
}

// ==================== 新增/编辑操作 ====================
function handleAdd() {
  isEdit.value = false
  Object.assign(formData, {
    building_no: "",
    unit_no: "",
    room_no: "",
    area: 0,
    owner_id: 0,
    id: undefined,
  })
  formRef.value?.clearValidate()
  dialogVisible.value = true
}

function handleEdit(row: House) {
  isEdit.value = true
  Object.assign(formData, {
    id: row.id,
    building_no: row.building_no,
    unit_no: row.unit_no,
    room_no: row.room_no,
    area: row.area,
    owner_id: row.owner_id,
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
      await updateHouse(formData.id, formData)
      ElMessage.success('更新成功')
    } else {
      await createHouse(formData)
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
function handleDelete(row: House) {
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
        await deleteHouse(row.id)
        ElMessage.success('删除成功')
        if (houseList.value.length === 1 && pagination.page > 1) {
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
.house_container {
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
