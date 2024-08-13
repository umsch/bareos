<script setup lang="ts">
import { ref, onMounted } from 'vue'

import { useWizzardStore } from '@/stores/wizzardStore'

const wizzardStore = useWizzardStore()

const columns = ref([
  {
    field: 'token',
    label: 'session'
  }
])
</script>

<template>
  <o-table
    v-model:selected="wizzardStore.selectedSession"
    :data="wizzardStore.sessions"
    :columns="columns"
    :loading="wizzardStore.isSessionsLoading"
    striped
    narrowed
  ></o-table>
</template>

<style scoped></style>
