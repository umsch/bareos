<script setup lang="ts">
import {onMounted, ref} from 'vue'
import {useRestoreClientStore} from '@/stores/restoreClientStore'
import Catalogs from '@/components/restore/CatalogSelector.vue'
import {useWizardStore} from '@/stores/wizardStore'
import type {Catalog} from '@/generated/config'
import {Job} from '@/generated/common'
import JobsTable from '@/components/restore/JobSelector.vue'
import type {RestoreSession} from '@/generated/restore'
import RestoreSessions from '@/components/restore/RestoreSessions.vue'
import FilesBrowser from '@/components/restore/FilesBrowser.vue'
import Clients from '@/components/restore/ClientSelector.vue'

const restoreClientStore = useRestoreClientStore()
const wizardStore = useWizardStore()

const sessions = ref<RestoreSession[]>([])
const isSessionsLoading = ref(false)

onMounted(async () => {
  await updateSessions()
})

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
        <div class="card">
          <div class="card-content">
            <Catalogs/>
            <JobsTable/>
            <Clients/>

            <o-button variant="primary" :disabled="!wizardStore.selectedClient" @click="wizardStore.runRestoreSession">
              Start
            </o-button>
          </div>
        </div>
      </div>
      <div class="column">
        <div class="card">
          <div class="card-content">
            <FilesBrowser/>
          </div>
        </div>
      </div>
    </div>
  </section>

  <section>
    <strong>DEBUG</strong>
    <pre>{{ wizardStore }}</pre>

  </section>

</template>
