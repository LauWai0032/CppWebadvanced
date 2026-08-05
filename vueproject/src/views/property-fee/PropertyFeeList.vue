<!--
  @file views/property-fee/PropertyFeeList.vue
  @brief 物业账单列表页面

  物业账单模块的主页面，提供增删改查（CRUD）功能。

  页面组成：
    1. 顶部搜索栏 + 新增按钮
    2. 中间数据表格
    3. 底部分页器
    4. 新增/编辑弹窗
    5. 删除确认

  @date 2025
-->

<template>
  <div class="property_fee_container">
    <!-- ==================== 顶部搜索栏 ==================== -->
    <el-card class="search-card" shadow="never">
      <el-form :inline="true" :model="searchForm" @submit.prevent="handleSearch">
        <el-form-item label="费用类型">
          <el-select
            v-model="searchForm.fee_type"
            placeholder="全部费用类型"
            clearable
            style="width: 140px"
          >
            <el-option label="物业费" value="property" />
            <el-option label="水费" value="water" />
            <el-option label="电费" value="electricity" />
            <el-option label="燃气费" value="gas" />
          </el-select>
        </el-form-item>
        <el-form-item label="状态">
          <el-select
            v-model="searchForm.status"
            placeholder="全部状态"
            clearable
            style="width: 140px"
          >
            <el-option label="未缴" value="unpaid" />
            <el-option label="已缴" value="paid" />
            <el-option label="逾期" value="overdue" />
          </el-select>
        </el-form-item>
        <el-form-item label="账单月份">
          <el-input
            v-model="searchForm.bill_month"
            placeholder="请输入账单月份"
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
        <el-button type="primary" :icon="Plus" @click="handleAdd">新增</el-button>
      </div>

      <el-table
        v-loading="loading"
        :data="feeList"
        border
        stripe
        style="width: 100%"
      >
        <el-table-column type="index" label="序号" width="60" align="center" />
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="house_id" label="房屋ID" width="100" />
        <el-table-column prop="fee_type" label="费用类型" width="100" />
        <el-table-column prop="amount" label="金额" width="100" />
        <el-table-column prop="bill_month" label="账单月份" width="120" />
        <el-table-column prop="status" label="状态" width="100" align="center">
          <template #default="{ row }">
            <el-tag>{{ row.status }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="payment_method" label="支付方式" width="100" />
        <el-table-column prop="paid_at" label="支付时间" width="180" />
        <el-table-column label="操作" width="220" fixed="right" align="center">
          <template #default="{ row }">
            <el-button
              v-if="row.status !== 'paid'"
              type="success"
              link
              @click="handlePay(row)"
            >缴费</el-button>
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
        <el-form-item label="房屋ID" prop="house_id">
          <el-input-number v-model="formData.house_id" :min="0" style="width: 100%" />
        </el-form-item>
        <el-form-item label="费用类型" prop="fee_type">
          <el-select v-model="formData.fee_type" placeholder="请选择费用类型" style="width: 100%">
            <el-option label="物业费" value="property" />
            <el-option label="水费" value="water" />
            <el-option label="电费" value="electricity" />
            <el-option label="燃气费" value="gas" />
          </el-select>
        </el-form-item>
        <el-form-item label="应缴金额" prop="amount">
          <el-input-number v-model="formData.amount" :min="0" style="width: 100%" />
        </el-form-item>
        <el-form-item label="账单月份" prop="bill_month">
          <el-input v-model="formData.bill_month" placeholder="请输入账单月份" />
        </el-form-item>
        <el-form-item label="状态" prop="status">
          <el-select v-model="formData.status" placeholder="请选择状态" style="width: 100%">
            <el-option label="未缴" value="unpaid" />
            <el-option label="已缴" value="paid" />
            <el-option label="逾期" value="overdue" />
          </el-select>
        </el-form-item>
        <el-form-item label="支付方式" prop="payment_method">
          <el-select v-model="formData.payment_method" placeholder="请选择支付方式" style="width: 100%">
            <el-option label="微信" value="wechat" />
            <el-option label="支付宝" value="alipay" />
            <el-option label="现金" value="cash" />
            <el-option label="银行转账" value="bank" />
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
 * @brief 物业账单列表页 - 脚本部分
 *
 * 使用 Vue 3 Composition API（<script setup> 语法糖）。
 */

import { ref, reactive, onMounted, computed } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { Search, Refresh, Plus, Edit, Delete } from '@element-plus/icons-vue'
import {
  getPropertyFeeList, createPropertyFee, updatePropertyFee, deletePropertyFee, payFee,
} from '@/api/propertyFee'
import type { PropertyFee, PropertyFeeForm, PropertyFeeQueryParams, PageResult } from '@/types'

// ==================== 搜索表单状态 ====================
const searchForm = reactive<PropertyFeeQueryParams>({
  page: 1,
  pageSize: 10,
  fee_type: "",
  status: "",
  bill_month: "",
})

// ==================== 表格数据与分页状态 ====================
const loading = ref(false)
const feeList = ref<PropertyFee[]>([])

const pagination = reactive({
  page: 1,
  pageSize: 10,
  total: 0,
})

// ==================== 弹窗表单状态 ====================
const dialogVisible = ref(false)
const formRef = ref<FormInstance>()
const formData = reactive<PropertyFeeForm>({
  house_id: 0,
  fee_type: "property",
  amount: 0,
  bill_month: "",
  status: "unpaid",
  payment_method: "wechat",
})

const isEdit = ref(false)
const submitLoading = ref(false)

const dialogTitle = computed(() => (isEdit.value ? '编辑物业账单' : '新增物业账单'))

const formRules: FormRules = {
  house_id: [
    { required: true, message: '请输入房屋ID', trigger: 'blur' },
  ],
  fee_type: [
    { required: true, message: '请输入费用类型', trigger: 'change' },
  ],
  amount: [
    { required: true, message: '请输入应缴金额', trigger: 'blur' },
  ],
  bill_month: [
    { required: true, message: '请输入账单月份', trigger: 'blur' },
  ],
  status: [
    { required: true, message: '请输入状态', trigger: 'change' },
  ],
  payment_method: [
    { required: true, message: '请输入支付方式', trigger: 'change' },
  ],
}

// ==================== 数据加载函数 ====================
async function loadList() {
  loading.value = true
  try {
    const params: PropertyFeeQueryParams = {
      page: pagination.page,
      pageSize: pagination.pageSize,
    }

    if (searchForm.fee_type) {
      params.fee_type = searchForm.fee_type
    }
    if (searchForm.status) {
      params.status = searchForm.status
    }
    if (searchForm.bill_month) {
      params.bill_month = searchForm.bill_month
    }

    const data = await getPropertyFeeList(params)
    feeList.value = data.list
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
  searchForm.fee_type = ''
  searchForm.status = ''
  searchForm.bill_month = ''
  pagination.page = 1
  loadList()
}

// ==================== 新增/编辑操作 ====================
function handleAdd() {
  isEdit.value = false
  Object.assign(formData, {
    house_id: 0,
    fee_type: "property",
    amount: 0,
    bill_month: "",
    status: "unpaid",
    payment_method: "wechat",
    id: undefined,
  })
  formRef.value?.clearValidate()
  dialogVisible.value = true
}

function handleEdit(row: PropertyFee) {
  isEdit.value = true
  Object.assign(formData, {
    id: row.id,
    house_id: row.house_id,
    fee_type: row.fee_type,
    amount: row.amount,
    bill_month: row.bill_month,
    status: row.status,
    payment_method: row.payment_method,
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
      await updatePropertyFee(formData.id, formData)
      ElMessage.success('更新成功')
    } else {
      await createPropertyFee(formData)
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
function handleDelete(row: PropertyFee) {
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
        await deletePropertyFee(row.id)
        ElMessage.success('删除成功')
        if (feeList.value.length === 1 && pagination.page > 1) {
          pagination.page -= 1
        }
        loadList()
      } catch (error) {
        console.error('删除失败:', error)
      }
    })
    .catch(() => {})
}

// ==================== 缴费操作 ====================

/**
 * @brief 缴费
 */
function handlePay(row: PropertyFee) {
  ElMessageBox.confirm(`确认缴费 ¥${row.amount} 吗？`, '缴费确认', {
    confirmButtonText: '确认缴费',
    cancelButtonText: '取消',
    type: 'warning',
  })
    .then(async () => {
      try {
        await payFee(row.id, 'wechat')
        ElMessage.success('缴费成功')
        loadList()
      } catch (error) {
        console.error('缴费失败:', error)
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
.property_fee_container {
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
