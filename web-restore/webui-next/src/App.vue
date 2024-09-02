<template>
  <MainLayout />
</template>

<script setup lang="ts">
import MainLayout from 'layouts/MainLayout.vue'
import 'json-bigint-patch'

defineOptions({
  name: 'App',
})
</script>
