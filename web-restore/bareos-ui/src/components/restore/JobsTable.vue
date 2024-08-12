<script setup lang="ts">
import type { Job } from '@/generated/common'
import { ref, watch } from 'vue'
import type { CatalogId } from '@/generated/config'
import { useRestoreClientStore } from '@/stores/restoreClientStore'

const props = defineProps<{
  catalog_id: CatalogId | undefined
}>()

const emit = defineEmits<{
  (e: 'update:selectedJob', job: Job): void
}>()

const restoreClientStore = useRestoreClientStore()

const selected = ref<Job | null>(null)
const jobs = ref<Job[]>([])
const isLoading = ref(false)

const updateJobs = async (catalog_id: CatalogId) => {
  isLoading.value = true
  try {
    jobs.value = await restoreClientStore.fetchJobs(catalog_id)
  } finally {
    isLoading.value = false
  }
}

watch(selected, (newValue) => {
  if (newValue) {
    emit('update:selectedJob', newValue)
  }
})

watch(
  () => props.catalog_id,  // Watch the catalog_id prop
  async (catalog_id) => {
    if (catalog_id) {
      await updateJobs(catalog_id)
    }
  }
)

const columns = ref([
  {
    field: 'jobid',
    label: 'Job ID',
    numeric: true
  }

])

</script>

<template>
  <!--  <o-field label="Jobs">-->
  <o-table
    v-model:selected="selected"
    :data="jobs" ,
    :columns="columns"
    :loading="isLoading"
    striped
    narrowed>
  </o-table>
  <!--  </o-field>-->
</template>

<style scoped>

</style>