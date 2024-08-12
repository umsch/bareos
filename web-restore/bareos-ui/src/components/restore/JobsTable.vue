<script setup lang="ts">
import type { Job } from '@/generated/common'
import { ref, watch } from 'vue'

const props = defineProps<{
  jobs: Job[]
  initialSelection: Job | null
  isLoading: boolean
}>()

const emit = defineEmits<{
  (e: 'update:selectedJob', job: Job): void
}>()

const selected = ref<Job | null>(props.initialSelection)

watch(selected, (newValue) => {
  if (newValue) {
    emit('update:selectedJob', newValue)
  }
})

const columns = ref([
  {
    field: "jobid",
    label: "Job ID",
    numeric: true
  }

])

</script>

<template>
<!--  <o-field label="Jobs">-->
    <o-table
      v-model:selected="selected"
      :data="jobs",
      :columns="columns"
      :loading="isLoading"
      striped
      narrowed>
    </o-table>
<!--  </o-field>-->
</template>

<style scoped>

</style>