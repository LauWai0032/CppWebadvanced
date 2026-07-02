<template>
  <div class="person-box">
    <h2>{{ person.course.yuwen }}</h2>
    <h2>{{ person.course.shuxue }}</h2>
    <h2>{{ person.course.yingyu }}</h2>
    <button @click="changeYuwen">语文 +1</button>
    <button @click="changeShuxue">数学 +1</button>
    <button @click="changeYingyu">英语 +1</button>
    <button @click="changeCourse">课程+1</button>
  </div>


</template>

<script setup lang="ts" name="Person">

import{ref,computed,reactive, watch} from "vue";
let name = ref("张三");
let firstName = ref("张");
let lastName = ref("三");
let fullName = computed(()=>{
  return firstName.value + " " + lastName.value;
});

let person = reactive({
  name:"李四",
  age: 30,
  course:{
    yuwen: 90,
    shuxue: 95,
    yingyu: 88
  }});
  function changeYuwen() {
    person.course.yuwen += 1;
  }
  function changeShuxue() {
    person.course.shuxue += 1;
  }
  function changeYingyu() {
    person.course.yingyu += 1;
  }
  function changeCourse() {
    person.course = {
      yuwen: 100,
      shuxue: 70,
      yingyu: 80
    };
  }
  watch(()=>{
    return person.course;
  },(newVal,oldVal)=>{
    console.log("课程发生了变化",newVal,oldVal);
  },{deep:true});


</script>

<style scoped>
/* 渐变蓝色 + 左上角定位 */
.person-box {
  /* 固定在左上角 */
  position: fixed;
  top: 20px;
  left: 20px;

  /* 渐变蓝色背景 */
  background: linear-gradient(135deg, #42b3ff 0%, #2181e2 100%);

  /* 样式美化 */
  padding: 20px 30px;
  border-radius: 10px;
  color: white;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
}
</style>