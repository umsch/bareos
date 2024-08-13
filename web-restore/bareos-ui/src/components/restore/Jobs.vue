<script setup lang="ts">
import { ref } from 'vue'

import {useWizardStore} from '@/stores/wizardStore'

const wizard = useWizardStore()

const columns = ref([
  {
    field: 'jobid',
    label: 'Job ID',
    numeric: true
  }
])
</script>

<template>
  <o-table
    v-model:selected="wizard.selectedJob"
    :data="wizard.jobs"
    :columns="columns"
    striped
    narrowed
  ></o-table>
</template>

<style scoped></style>
