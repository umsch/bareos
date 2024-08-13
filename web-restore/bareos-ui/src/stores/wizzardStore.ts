import { defineStore } from 'pinia'
import type { Catalog } from '@/generated/config'
import { ref } from 'vue'
import type { Client, Job } from '@/generated/common'
import type { RestoreSession } from '@/generated/restore'
import { useRestoreClientStore } from '@/stores/restoreClientStore'

export const useWizzardStore = defineStore('wizzard', () => {
  const restoreClient = useRestoreClientStore()

  const selectedCatalog = ref<Catalog | null>(null)
  const selectedClient = ref<Client | null>(null)
  const selectedJob = ref<Job | null>(null)

  const selectedSession = ref<RestoreSession | null>(null)
  const sessions = ref<RestoreSession[]>([])
  const isSessionsLoading = ref(false)

  const updateSessions = async () => {
    try {
      isSessionsLoading.value = true
      sessions.value = await restoreClient.fetchSessions()
    } finally {
      isSessionsLoading.value = false
    }
  }

  return {
    selectedCatalog,
    selectedClient,
    selectedJob,
    updateSessions,
    sessions,
    selectedSession,
    isSessionsLoading
  }
})
