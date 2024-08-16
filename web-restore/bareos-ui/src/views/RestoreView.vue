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
      <div class="column">
        <div class="card">
          <div class="card-header">
            <p class="card-header-title">start new restore session</p>
          </div>
          <div class="card-content">
            <Catalogs />

            <JobsTable :catalog_id="wizardStore.selectedCatalog?.id" @update:selectedJob="setJob" />
          </div>
          <footer class="card-footer">
            <o-button
              class="card-footer-item"
              :disabled="!wizardStore.selectedJob"
              variant="primary"
              :label="
                'start new restore session' +
                (wizardStore.selectedJob ? ' for job ' + wizardStore.selectedJob.jobid : '')
              "
              @click="wizardStore.startRestoreSession"
            />
          </footer>
        </div>
      </div>

      <div class="column">
        <div class="card">
          <div class="card-header">
            <p class="card-header-title">select existing restore session</p>
          </div>
          <div class="card-content">
            <RestoreSessions />
          </div>
        </div>
      </div>
    </div>
  </section>

  <section class="section">
    <div class="card">
      <div class="card-header">
        <p class="card-header-title">restore session</p>
      </div>
      <div class="card-content">
        <FilesBrowser />
      </div>
      <div class="card-footer">
        <Clients />
        <o-button
          variant="primary"
          class="card-footer-item"
          :disabled="!wizardStore.selectedClient"
        >
          Start
        </o-button>
      </div>
    </div>
  </section>
</template>
