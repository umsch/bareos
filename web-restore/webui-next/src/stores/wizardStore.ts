import { defineStore } from 'pinia'
import type { Catalog } from 'src/generated/config'
import { onBeforeMount, ref, watch } from 'vue'
import type { Client, Job } from 'src/generated/common'
import type { RestoreSession, File, SessionState } from 'src/generated/restore'
import { useRestoreClientStore } from 'src/stores/restoreClientStore'
import { find, first, isEmpty, isEqual, reverse } from 'lodash'

export const useWizardStore = defineStore('wizard', () => {
  const restoreClient = useRestoreClientStore()

  const selectCatalogFromState = (state: SessionState, catalogs: Catalog[]) => {
    if (catalogs.length > 0 && state?.start?.catalog) {
      selectedCatalog.value =
        find(catalogs, (c) => isEqual(c.id, state.start?.catalog)) ?? null
    }
  }

  const selectJobFromState = (state: SessionState, jobs: Job[]) => {
    if (jobs.length > 0 && state.start?.backupJob) {
      selectedJob.value =
        find(jobs, (j) => isEqual(j.jobid, state.start?.backupJob?.jobid)) ??
        null
    }
  }

  onBeforeMount(async () => {
    console.log('onBeforeMount - store')
    await updateSessions()

    if (!selectedSession.value && sessions.value.length > 0) {
      selectedSession.value = first(sessions.value)!
    }

    if (selectedSession.value) {
      await updateState()
    }

    await updateCatalogs()
    if (sessionState.value && catalogs.value) {
      selectCatalogFromState(sessionState.value, catalogs.value)
    }

    if (!selectedCatalog.value && catalogs.value && catalogs.value.length > 0) {
      selectedCatalog.value = first(catalogs.value)!
    }

    await updateJobs()
    if (sessionState.value && catalogs.value) {
      selectJobFromState(sessionState.value, jobs.value)
    }
  })

  // clients
  const catalogs = ref<Catalog[]>([])
  const selectedCatalog = ref<Catalog | null>(null)
  const updateCatalogs = async () => {
    console.debug('fetching catalogs')
    catalogs.value = await restoreClient.fetchCatalogs()
  }

  // jobs
  const selectedJob = ref<Job | null>(null)
  const jobs = ref<Job[]>([])
  const updateJobs = async () => {
    const catalog_id = selectedCatalog.value?.id
    if (catalog_id) {
      console.debug('fetching jobs from catalog', catalog_id)
      jobs.value = await restoreClient.fetchJobs(catalog_id)
    }
  }

  // sessions
  const selectedSession = ref<RestoreSession | null>(null)
  const sessions = ref<RestoreSession[]>([])

  const updateSessions = async () => {
    console.debug('fetching sessions')
    sessions.value = await restoreClient.fetchSessions()
  }

  // client
  const clients = ref<Client[]>([])
  const selectedClient = ref<Client | null>(null)
  const updateClients = async (catalog: Catalog) => {
    clients.value = await restoreClient.fetchClients(catalog)
  }

  // logics

  watch(selectedCatalog, async (catalog) => {
    console.debug('selected catalog changed', catalog)

    if (!catalog) {
      console.debug('catalog was unset')
      selectedJob.value = null
      jobs.value = []

      selectedClient.value = null
      clients.value = []

      return
    }

    if (catalog?.id) {
      await updateJobs()
      await updateClients(catalog)
    }
  })

  watch(selectedJob, async (job) => {
    if (isEqual(sessionState.value?.start?.backupJob, selectedJob.value)) {
      console.debug('session already running for job', selectedJob.value)
      return
    }

    await cleanUpSessions()
    if (job) {
      await createRestoreSession()
      await updateSessions()
    }
    await updateState()
  })

  const cleanUpSessions = async () => {
    selectedSession.value = null

    if (!isEmpty(sessions.value)) {
      for (const session of sessions.value) {
        await restoreClient.deleteSession(session)
      }
    }

    await updateSessions()
  }

  const createRestoreSession = async () => {
    console.debug(
      '---- starting restore session',
      selectedJob.value,
      sessionState.value?.start?.backupJob
    )

    if (!selectedJob.value || !selectedCatalog.value) {
      return
    }

    const session = await restoreClient.createSession(
      selectedCatalog.value,
      selectedJob.value
    )
    await updateSessions()
    selectedSession.value = session!
  }

  const runRestoreSession = async () => {
    if (!selectedClient.value) {
      throw new Error('No client selected')
    }

    if (!selectedSession.value) {
      throw new Error('No session selected')
    }

    await restoreClient.runSession(selectedSession.value!, selectedClient.value)
  }

  const deleteRestoreSession = async () => {
    if (!selectedSession.value) {
      return
    }

    await restoreClient.deleteSession(selectedSession.value)
    selectedSession.value = null
    await updateSessions()
  }

  const files = ref<File[]>([])
  const currentDirectory = ref<File[] | null>([])

  watch(selectedSession, async (session) => {
    console.debug('selected session changed', session)
    console.debug('updating path and files')

    if (session) {
      const currentPath = await restoreClient.currentDirectory(session)
      currentDirectory.value = reverse(currentPath!)
      files.value = await restoreClient.fetchFiles(session)
    } else {
      currentDirectory.value = null
      files.value = []
    }
    await updateState()
  })

  const changeDirectory = async (path: File) => {
    if (!selectedSession.value) {
      throw new Error('No session selected')
    }

    const currentPath = await restoreClient.changeDirectory(
      selectedSession.value,
      path
    )
    currentDirectory.value = reverse(currentPath!)
    files.value = await restoreClient.fetchFiles(selectedSession.value)
  }

  const updateMarkedStatus = async (file: File) => {
    if (!selectedSession.value) {
      throw new Error('No session selected')
    }

    await restoreClient.changeMarkedStatus(
      selectedSession.value,
      file,
      file.marked
    )
    await updateState()
  }

  const sessionState = ref<SessionState | null>(null)

  const updateState = async () => {
    if (!selectedSession.value) {
      sessionState.value = null
      return
    }

    sessionState.value = await restoreClient.fetchState(selectedSession.value)
    console.debug('new State: ', sessionState.value)
  }

  return {
    updateCatalogs,
    catalogs,
    selectedCatalog,
    clients,
    selectedClient,
    jobs,
    selectedJob,
    updateSessions,
    sessions,
    selectedSession,
    createRestoreSession,
    runRestoreSession,
    deleteRestoreSession,
    files,
    currentDirectory,
    changeDirectory,
    updateMarkedStatus,
    sessionState,
  }
})
