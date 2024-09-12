<template>
  <MainLayout />
</template>

<script setup lang="ts">
import 'json-bigint-patch'

import MainLayout from 'layouts/MainLayout.vue'

defineOptions({
  name: 'App',
})
</script>
