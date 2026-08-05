<!--
  @file views/parking/ParkingSpotList.vue
  @brief 车位管理列表页面

  车位管理模块的主页面，提供增删改查（CRUD）功能。

  页面组成：
    1. 顶部搜索栏 + 新增按钮
    2. 中间数据表格
    3. 底部分页器
    4. 新增/编辑弹窗
    5. 删除确认

  @date 2025
-->

<template>
  <div class="parking_container">
    <!-- ==================== 顶部搜索栏 ==================== -->
    <el-card class="search-card" shadow="never">
      <el-form :inline="true" :model="searchForm" @submit.prevent="handleSearch">
        <el-form-item label="车位编号">
          <el-input
            v-model="searchForm.spot_number"
            placeholder="请输入车位编号"
            clearable
            style="width: 180px"
          />
        </el-form-item>
        <el-form-item label="区域">
          <el-input
            v-model="searchForm.zone"
            placeholder="请输入区域"
            clearable
            style="width: 180px"
          />
        </el-form-item>
        <el-form-item label="类型">
          <el-select
            v-model="searchForm.type"
            placeholder="全部类型"
            clearable
            style="width: 140px"
          >
            <el-option label="固定" value="fixed" />
            <el-option label="临时" value="temporary" />
          </el-select>
        </el-form-item>
        <el-form-item label="是否占用">
          <el-select
            v-model="searchForm.is_occupied"
            placeholder="全部是否占用"
            clearable
            style="width: 140px"
          >
            <el-option label="空闲" value="0" />
            <el-option label="占用" value="1" />
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
        <el-button type="primary" :icon="Plus" @click="handleAdd">新增车位</el-button>
      </div>

      <el-table
        v-loading="loading"
        :data="parkingSpotList"
        border
        stripe
        style="width: 100%"
      >
        <el-table-column type="index" label="序号" width="60" align="center" />
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="spot_number" label="车位编号" width="120" />
        <el-table-column prop="zone" label="区域" width="100" />
        <el-table-column prop="type" label="类型" width="100" />
        <el-table-column prop="current_vehicle_id" label="当前车辆ID" width="120" />
        <el-table-column prop="is_occupied" label="是否占用" width="100" align="center">
          <template #default="{ row }">
            <el-tag :type="row.is_occupied === 1 ? 'warning' : 'success'">
              {{ row.is_occupied === 1 ? '占用' : '空闲' }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="updated_at" label="更新时间" width="180" />
        <el-table-column label="操作" width="220" fixed="right" align="center">
          <template #default="{ row }">
            <el-button
              v-if="row.is_occupied === 1"
              type="warning"
              link
              @click="handleLeave(row)"
            >离场</el-button>
            <el-button
              v-else
              type="success"
              link
              @click="handlePark(row)"
            >入场</el-button>
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
        <el-form-item label="车位编号" prop="spot_number">
          <el-input v-model="formData.spot_number" placeholder="请输入车位编号" />
        </el-form-item>
        <el-form-item label="区域" prop="zone">
          <el-input v-model="formData.zone" placeholder="请输入区域" />
        </el-form-item>
        <el-form-item label="类型" prop="type">
          <el-select v-model="formData.type" placeholder="请选择类型" style="width: 100%">
            <el-option label="固定" value="fixed" />
            <el-option label="临时" value="temporary" />
          </el-select>
        </el-form-item>
        <el-form-item label="当前车辆ID" prop="current_vehicle_id">
          <el-input-number v-model="formData.current_vehicle_id" :min="0" style="width: 100%" />
        </el-form-item>
        <el-form-item label="是否占用" prop="is_occupied">
          <el-radio-group v-model="formData.is_occupied">
            <el-radio :value="1">是</el-radio>
            <el-radio :value="0">否</el-radio>
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
 * @brief 车位管理列表页 - 脚本部分
 *
 * 使用 Vue 3 Composition API（<script setup> 语法糖）。
 */

import { ref, reactive, onMounted, computed } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { Search, Refresh, Plus, Edit, Delete } from '@element-plus/icons-vue'
import {
  getParkingSpotList, createParkingSpot, updateParkingSpot, deleteParkingSpot, parkVehicle, leaveVehicle,
} from '@/api/parking'
import type { ParkingSpot, ParkingSpotForm, ParkingSpotQueryParams, PageResult } from '@/types'

// ==================== 搜索表单状态 ====================
const searchForm = reactive<ParkingSpotQueryParams>({
  page: 1,
  pageSize: 10,
  spot_number: "",
  zone: "",
  type: "",
  is_occupied: "",
})

// ==================== 表格数据与分页状态 ====================
const loading = ref(false)
const parkingSpotList = ref<ParkingSpot[]>([])

const pagination = reactive({
  page: 1,
  pageSize: 10,
  total: 0,
})

// ==================== 弹窗表单状态 ====================
const dialogVisible = ref(false)
const formRef = ref<FormInstance>()
const formData = reactive<ParkingSpotForm>({
  spot_number: "",
  zone: "",
  type: "fixed",
  current_vehicle_id: 0,
  is_occupied: 0,
})

const isEdit = ref(false)
const submitLoading = ref(false)

const dialogTitle = computed(() => (isEdit.value ? '编辑车位' : '新增车位'))

const formRules: FormRules = {
  spot_number: [
    { required: true, message: '请输入车位编号', trigger: 'blur' },
  ],
  type: [
    { required: true, message: '请输入类型', trigger: 'change' },
  ],
  is_occupied: [
    { required: true, message: '请输入是否占用', trigger: 'change' },
  ],
}

// ==================== 数据加载函数 ====================
async function loadList() {
  loading.value = true
  try {
    const params: ParkingSpotQueryParams = {
      page: pagination.page,
      pageSize: pagination.pageSize,
    }

    if (searchForm.spot_number) {
      params.spot_number = searchForm.spot_number
    }
    if (searchForm.zone) {
      params.zone = searchForm.zone
    }
    if (searchForm.type) {
      params.type = searchForm.type
    }
    if (searchForm.is_occupied) {
      params.is_occupied = searchForm.is_occupied
    }

    const data = await getParkingSpotList(params)
    parkingSpotList.value = data.list
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
  searchForm.spot_number = ''
  searchForm.zone = ''
  searchForm.type = ''
  searchForm.is_occupied = ''
  pagination.page = 1
  loadList()
}

// ==================== 新增/编辑操作 ====================
function handleAdd() {
  isEdit.value = false
  Object.assign(formData, {
    spot_number: "",
    zone: "",
    type: "fixed",
    current_vehicle_id: 0,
    is_occupied: 0,
    id: undefined,
  })
  formRef.value?.clearValidate()
  dialogVisible.value = true
}

function handleEdit(row: ParkingSpot) {
  isEdit.value = true
  Object.assign(formData, {
    id: row.id,
    spot_number: row.spot_number,
    zone: row.zone,
    type: row.type,
    current_vehicle_id: row.current_vehicle_id,
    is_occupied: row.is_occupied,
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
      await updateParkingSpot(formData.id, formData)
      ElMessage.success('更新成功')
    } else {
      await createParkingSpot(formData)
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
function handleDelete(row: ParkingSpot) {
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
        await deleteParkingSpot(row.id)
        ElMessage.success('删除成功')
        if (parkingSpotList.value.length === 1 && pagination.page > 1) {
          pagination.page -= 1
        }
        loadList()
      } catch (error) {
        console.error('删除失败:', error)
      }
    })
    .catch(() => {})
}

// ==================== 入场/离场操作 ====================

/**
 * @brief 车辆入场
 */
function handlePark(row: ParkingSpot) {
  ElMessageBox.prompt('请输入车辆ID', '车辆入场', {
    confirmButtonText: '确认入场',
    cancelButtonText: '取消',
    inputPattern: /^\d+$/,
    inputErrorMessage: '请输入有效的车辆ID',
  })
    .then(async ({ value }) => {
      try {
        await parkVehicle(row.id, parseInt(value))
        ElMessage.success('入场成功')
        loadList()
      } catch (error) {
        console.error('入场失败:', error)
      }
    })
    .catch(() => {})
}

/**
 * @brief 车辆离场
 */
function handleLeave(row: ParkingSpot) {
  ElMessageBox.confirm('确认车辆离场吗？', '提示', {
    confirmButtonText: '确认离场',
    cancelButtonText: '取消',
    type: 'warning',
  })
    .then(async () => {
      try {
        await leaveVehicle(row.id)
        ElMessage.success('离场成功')
        loadList()
      } catch (error) {
        console.error('离场失败:', error)
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
.parking_container {
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
