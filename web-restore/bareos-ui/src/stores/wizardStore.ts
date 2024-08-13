import { defineStore } from 'pinia'
import type { Catalog, CatalogId } from '@/generated/config'
import { onMounted, ref, watch } from 'vue'
import type { Client, Job } from '@/generated/common'
import type { RestoreSession } from '@/generated/restore'
import { useRestoreClientStore } from '@/stores/restoreClientStore'

export const useWizardStore = defineStore('wizard', () => {
  const restoreClient = useRestoreClientStore()

  onMounted(async () => {
    await updateCatalogs()
    await updateSessions()
  })

  // clients
  const catalogs = ref<Catalog[]>([])
  const isCatalogsLoading = ref(false)
  const selectedCatalog = ref<Catalog | null>(null)
  const updateCatalogs = async () => {
    isCatalogsLoading.value = true
    try {
      console.debug('fetching catalogs')
      catalogs.value = await restoreClient.fetchCatalogs()
    } finally {
      isCatalogsLoading.value = false
    }
  }

  // jobs
  const selectedJob = ref<Job | null>(null)
  const jobs = ref<Job[]>([])
  const isJobsLoading = ref(false)
  const updateJobs = async (catalog_id: CatalogId) => {
    isJobsLoading.value = true
    try {
      console.debug('fetching jobs from catalog', catalog_id)
      jobs.value = await restoreClient.fetchJobs(catalog_id)
    } finally {
      isJobsLoading.value = false
    }
  }

  // sessions
  const selectedSession = ref<RestoreSession | null>(null)
  const sessions = ref<RestoreSession[]>([])
  const isSessionsLoading = ref(false)

  const updateSessions = async () => {
    try {
      console.debug('fetching sessions')
      isSessionsLoading.value = true
      sessions.value = await restoreClient.fetchSessions()
    } finally {
      isSessionsLoading.value = false
    }
  }

  // client
  const selectedClient = ref<Client | null>(null)

  // logics

  watch(selectedCatalog, async (catalog) => {
    console.debug('selected catalog changed', catalog)
    if (catalog?.id) {
      await updateJobs(catalog.id)
    }
  })

  const startRestoreSession = async () => {
    console.debug('starting restore session', selectedJob.value?.jobid)

    if (!selectedJob.value) {
      throw new Error('No job selected')
    }

    const session = await restoreClient.createSession(selectedJob.value)
    await updateSessions()
    selectedSession.value = session!
  }

  return {
    updateCatalogs,
    catalogs,
    selectedCatalog,
    selectedClient,
    updateJobs,
    isJobsLoading,
    jobs,
    selectedJob,
    updateSessions,
    isSessionsLoading,
    sessions,
    selectedSession,
    startRestoreSession
  }
})
