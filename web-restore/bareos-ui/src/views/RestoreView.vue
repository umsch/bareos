<script setup lang="ts">
import { onMounted, ref } from 'vue'
import { useRestoreClientStore } from '@/stores/restoreClientStore'
import CatalogDropdown from '@/components/restore/Catalogs.vue'
import { useWizzardStore } from '@/stores/wizzardStore'
import type { Catalog } from '@/generated/config'
import { Job } from '@/generated/common'
import JobsTable from '@/components/restore/Jobs.vue'
import type { RestoreSession } from '@/generated/restore'
import RestoreSessions from '@/components/restore/RestoreSessions.vue'
import { OField } from '@oruga-ui/oruga-next'

const restoreClientStore = useRestoreClientStore()
const wizzardStore = useWizzardStore()

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

const setSession = async (session: RestoreSession) => {
  wizzardStore.selectedSession = session
  console.log('selectedSession', session.token)
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
            <p class="card-header-title">
              start new restore session
            </p>
          </div>
          <div class="card-content">
            <o-field label="Catalog:" horizontal>
              <CatalogDropdown
                :catalogs="restoreClientStore.catalogs"
                :initialSelection="wizzardStore.selectedCatalog"
                @update:selectedCatalog="setCatalog"
              />
            </o-field>

            <JobsTable
              :catalog_id="wizzardStore.selectedCatalog?.id"
              @update:selectedJob="setJob" />
          </div>
          <footer class="card-footer">
            <o-button
              :disabled="!wizzardStore.selectedJob"
              variant="primary"
              label="start restore session"
              @click=""
            />
          </footer>
        </div>
      </div>

      <div class="column">
        <div class="card">
          <div class="card-header">
            <p class="card-header-title">
              select existing restore session
            </p>
          </div>
          <div class="card-content">
            <RestoreSessions
            />
          </div>
          <footer class="card-footer">
            <o-button
              :disabled="!wizzardStore.selectedJob"
              variant="primary"
              label="start restore session"
              @click=""
            />
          </footer>
        </div>

      </div>

    </div>
  </section>

</template>
