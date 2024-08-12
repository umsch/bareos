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


onMounted(async () => {
  await updateSessions()
})

const setCatalog = async (catalog: Catalog) => {
  wizzardStore.selectedCatalog = catalog
}

const setJob = async (job: Job) => {
  wizzardStore.selectedJob = job
  console.log('selectedJob', job.jobid)
}

const updateSessions = async () => {
  try {
    isSessionsLoading.value = true
    sessions.value = await restoreClientStore.fetchSessions()
  } finally {
    isSessionsLoading.value = false
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
      :catalog_id="wizzardStore.selectedCatalog?.id"
      @update:selectedJob="setJob"
    />
  </section>

<!--  <section class="section">-->
<!--    <SessionsTable-->
<!--      :sessions="sessions"-->
<!--      @update:selectedSession="setSession"-->
<!--      @update:deleteSession="deleteSession"-->
<!--      :isLoading="isJobsLoading"-->
<!--    />-->
<!--  </section>-->


</template>
