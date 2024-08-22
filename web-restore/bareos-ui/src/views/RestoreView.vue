<script setup lang="ts">
import { onMounted, ref } from 'vue'
import { useRestoreClientStore } from '@/stores/restoreClientStore'
import Catalogs from '@/components/restore/Catalogs.vue'
import { useWizardStore } from '@/stores/wizardStore'
import type { Catalog } from '@/generated/config'
import { Job } from '@/generated/common'
import JobsTable from '@/components/restore/Jobs.vue'
import type { RestoreSession } from '@/generated/restore'
import RestoreSessions from '@/components/restore/RestoreSessions.vue'
import FilesBrowser from '@/components/restore/FilesBrowser.vue'
import Clients from '@/components/restore/Clients.vue'

const restoreClientStore = useRestoreClientStore()
const wizardStore = useWizardStore()

const sessions = ref<RestoreSession[]>([])
const isSessionsLoading = ref(false)

onMounted(async () => {
  await updateSessions()
})

const setJob = async (job: Job) => {
  wizardStore.selectedJob = job
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
    <div class="columns">
      <div class="column is-narrow">
        <Catalogs />
        <JobsTable v-if="wizardStore.selectedCatalog" :catalog_id="wizardStore.selectedCatalog?.id"
                   @update:selectedJob="setJob" />
        <o-button
          :disabled="!wizardStore.selectedJob"
          variant="primary"
          :label="
                'start new restore session' +
                (wizardStore.selectedJob ? ' for job ' + wizardStore.selectedJob.jobid : '')
              "
          @click="wizardStore.createRestoreSession"
        />
        <RestoreSessions />
        <Clients />
      </div>
      <div class="column">
        <FilesBrowser />
        <o-button
          variant="primary"
          :disabled="!wizardStore.selectedClient"
          @click="wizardStore.runRestoreSession"
        >
          Start
        </o-button>
      </div>
    </div>
  </section>

 </template>
