import { defineStore } from 'pinia'
import { onBeforeMount, ref } from 'vue'
import { GrpcWebFetchTransport } from '@protobuf-ts/grpcweb-transport'
import { ConfigClient, type IConfigClient } from '@/generated/config.client'
import type { Catalog, CatalogId } from '@/generated/config'
import { DatabaseClient, type IDatabaseClient } from '@/generated/database.client'
import type { Client, Job } from '@/generated/common'
import { type IRestoreClient, RestoreClient } from '@/generated/restore.client'
import { type File, FileType, MarkAction, type RestoreSession } from '@/generated/restore'

export const useRestoreClientStore = defineStore('restore-client', () => {
  const transport = ref(new GrpcWebFetchTransport({ baseUrl: 'http://127.0.0.1:9090' }))

  const configClient = ref<IConfigClient | null>(null)
  const databaseClient = ref<IDatabaseClient | null>(null)
  const restoreClient = ref<IRestoreClient | null>(null)

  const catalogs = ref<Catalog[]>([])

  onBeforeMount(async () => {
    console.log('Initializing clients')
    await initializeClient()
  })

  const initializeClient = async () => {
    if (!configClient.value) {
      configClient.value = new ConfigClient(transport.value)
    }

    if (!databaseClient.value) {
      databaseClient.value = new DatabaseClient(transport.value)
    }

    if (!restoreClient.value) {
      restoreClient.value = new RestoreClient(transport.value)
    }

    console.log('Clients initialized')
  }

  const fetchCatalogs = async () => {
    if (!configClient.value) {
      console.error('configClient not initialized')
      return []
    }

    const response = await configClient.value.listCatalogs({})
    return response.response.catalogs
  }

  const fetchClients = async (catalog: Catalog) => {
    if (!configClient.value) {
      console.error('configClient not initialized')
      return []
    }

    const clients: Client[] = []

    const call = databaseClient.value?.listClients({ catalog: catalog.id })
    for await (const client of call?.responses!) {
      clients.push(client.client!)
    }

    return clients
  }

  const fetchJobs = async (catalog_id: CatalogId) => {
    if (!configClient.value) {
      console.error('configClient not initialized')
      return []
    }

    const call = databaseClient.value?.listJobs({ catalog: catalog_id })
    let jobs: Job[] = []
    for await (const job of call?.responses!) {
      jobs.push(job.job!)
    }

    return jobs
  }

  const fetchSessions = async () => {
    const response = await restoreClient.value?.listSessions({})
    return response?.response.sessions!
  }

  const createSession = async (job: Job) => {
    const response = await restoreClient.value?.begin({ backupJob: job, findJobChain: false })
    return response?.response.session
  }

  const deleteSession = async (session: RestoreSession) => {
    await restoreClient.value?.cancel({ session })
  }

  const runSession = async (session: RestoreSession, client: Client) => {
    console.debug("running session:", session, client)
    try {
      await restoreClient.value?.run({
        session,
        restoreOptions: {
          restoreClient: client
        }
      })
    }
    catch (e) {
      console.error(e)
    }

    console.debug("session end")
  }

  const fetchFiles = async (session: RestoreSession) => {
    console.debug('fetching files for session', session)
    const files: File[] = []
    try {
      const call = restoreClient.value?.listFiles({ session: session })
      console.debug('call', call)
      for await (const file of call?.responses!) {
        console.debug('file', file)
        files.push(file)
      }
    } catch (e) {
      console.error('error', e)
    }
    return files
  }

  const changeDirectory = async (session: RestoreSession, path: File) => {
    if (path.type !== FileType.DIRECTORY && path.type !== FileType.DIRECTORY_NOT_BACKED_UP) {
      throw new Error('Invalid file type. File type must be a directory')
    }

    const response = await restoreClient.value?.changeDirectory({
      session: session,
      directory: path.id
    })
    return response?.response.currentDirectory?.path
  }

  const changeMarkedStatus = async (session: RestoreSession, file: File, mark: boolean) => {
    console.debug('changing marked status: ', file, mark)

    await restoreClient.value?.changeMarkedStatus({
      session: session,
      action: mark ? MarkAction.MARK : MarkAction.UNMARK,
      affectedId: file.id!,
      recursive: file.type === FileType.DIRECTORY || file.type === FileType.DIRECTORY_NOT_BACKED_UP
    })
  }

  return {
    transport,
    catalogs,
    fetchCatalogs,
    fetchClients,
    fetchJobs,
    fetchSessions,
    createSession,
    deleteSession,
    runSession,
    fetchFiles,
    changeDirectory,
    changeMarkedStatus
  }
})
