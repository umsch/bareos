<script setup lang="ts">

import { onMounted, ref } from 'vue'
import { useRestoreClientStore } from '@/stores/restoreClientStore'
import CatalogDropdown from '@/components/restore/CatalogDropdown.vue'
import { useWizzardStore } from '@/stores/wizzardStore'
import type { Catalog } from '@/generated/config'
import { Job } from '@/generated/common'
import JobsTable from '@/components/restore/JobsTable.vue'
import type { RestoreSession } from '@/generated/restore'

const restoreClientStore = useRestoreClientStore()
const wizzardStore = useWizzardStore()

const jobs = ref<Job[]>([])
const isJobsLoading = ref(false)

const sessions = ref<RestoreSession[]>([])
const isSessionsLoading = ref(false)


onMounted(() => {
  restoreClientStore.initializeClient()
})

const setCatalog = async (catalog: Catalog) => {
  wizzardStore.selectedCatalog = catalog

  try {
    isJobsLoading.value = true
    jobs.value = await restoreClientStore.fetchJobs(catalog)
  } finally {
    isJobsLoading.value = false
  }
}

</script>

<template>
  <section class="section">
    <CatalogDropdown
      :catalogs="restoreClientStore.catalogs"
      :initialSelection="wizzardStore.selectedCatalog"
      @update:selectedCatalog="setCatalog"
    />
  </section>
  <section class="section">
    <JobsTable
      :jobs="jobs"
      :initialSelection="wizzardStore.selectedJob"
      @update:selectedJob="wizzardStore.selectedJob = $event"
      :isLoading="isJobsLoading"
    />
  </section>

</template>
