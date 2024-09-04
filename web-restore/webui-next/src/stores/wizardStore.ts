import { defineStore } from 'pinia'
import type { Catalog } from 'src/generated/config'
import { onBeforeMount, ref, watch } from 'vue'
import type { Client, Job } from 'src/generated/database'
import type { Client as ConfigClient } from 'src/generated/config'
import type { RestoreSession, File, SessionState } from 'src/generated/restore'
import { useRestoreClientStore } from 'src/stores/restoreClientStore'
import { find, first, isEmpty, isEqual, reverse } from 'lodash'
import { useConfigStore } from 'stores/configStore'
import { useDatabaseStore } from 'stores/databaseStore'

export const useWizardStore = defineStore('wizard', () => {
  const restoreClient = useRestoreClientStore()
  const configClient = useConfigStore()
  const databaseClient = useDatabaseStore()

  const selectCatalogFromState = (state: SessionState, catalogs: Catalog[]) => {
    if (catalogs.length > 0 && state?.start?.catalog) {
      selectedCatalog.value =
        find(catalogs, (c) => isEqual(c.id, state.start?.catalog)) ?? null
    }
  }

  const selectJobFromState = (state: SessionState, jobs: Job[]) => {
    if (jobs.length > 0 && state.start?.backupJob) {
      selectedJob.value =
        find(jobs, (j) => isEqual(j.id, state.start?.backupJob?.id)) ?? null
    }
  }

  onBeforeMount(async () => {
    console.log('onBeforeMount - store')
    await updateSessions()
    await updateConfigClients()
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

    await updateJobs(selectedCatalog.value!, null)
    if (sessionState.value && catalogs.value) {
      selectJobFromState(sessionState.value, jobs.value)
    }
  })

  // clients
  const catalogs = ref<Catalog[]>([])
  const selectedCatalog = ref<Catalog | null>(null)
  const updateCatalogs = async () => {
    console.debug('fetching catalogs')
    catalogs.value = await configClient.getConfigCatalogs()
  }

  // jobs
  const selectedJob = ref<Job | null>(null)
  const jobs = ref<Job[]>([])
  const updateJobs = async (catalog: Catalog, filter: Client | null) => {
    const catalog_id = catalog.id
    if (catalog_id && filter) {
      console.debug('fetching jobs from catalog', catalog_id)
      jobs.value = await databaseClient.listJobs(catalog_id, filter!)
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
  const updateClients = async (catalog: Catalog) => {
    clients.value = await databaseClient.listClients(catalog)
  }

  const selectedSourceClient = ref<Client | null>(null)

  watch(selectedSourceClient, async (sourceClient) => {
    await updateJobs(selectedCatalog.value!, sourceClient)
    selectedJob.value = null
  })

  // logics

  watch(selectedCatalog, async (catalog) => {
    console.debug('selected catalog changed', catalog)

    if (!catalog) {
      console.debug('catalog was unset')
      selectedJob.value = null
      jobs.value = []

      clients.value = []

      return
    }

    if (catalog?.id) {
      await updateJobs(selectedCatalog.value!, selectedSourceClient.value)
      await updateClients(catalog)
    }
  })

  watch(selectedJob, async (job) => {
    console.debug('backup job changed to', job)

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
    if (!sessionState.value?.restoreOptions?.restoreClient) {
      throw new Error('No client selected')
    }

    if (!selectedSession.value) {
      throw new Error('No session selected')
    }

    await restoreClient.runSession(selectedSession.value!)
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

  const pushRestoreOptions = async () => {
    if (selectedSession.value && sessionState.value) {
      await restoreClient.pushRestoreOptions(
        selectedSession.value,
        sessionState.value?.restoreOptions ?? {}
      )
    }
  }

  watch(
    () => sessionState.value?.restoreOptions,
    async () => {
      console.debug(
        'pushing restore options',
        sessionState.value?.restoreOptions
      )
      await pushRestoreOptions()
    }
  )

  const configClients = ref<ConfigClient[]>([])

  const updateConfigClients = async () => {
    console.debug('fetching config clients')
    configClients.value = await configClient.getConfigClients()
  }

  return {
    updateCatalogs,
    catalogs,
    selectedCatalog,
    clients,
    selectedSourceClient,
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
    pushRestoreOptions,
    configClients,
  }
})
