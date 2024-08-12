import { defineStore } from 'pinia'
import { ref, onBeforeMount } from 'vue'
import { GrpcWebFetchTransport } from '@protobuf-ts/grpcweb-transport'
import { ConfigClient, type IConfigClient } from '@/generated/config.client'
import type { Catalog, CatalogId } from '@/generated/config'
import { DatabaseClient, type IDatabaseClient } from '@/generated/database.client'
import type { Job } from '@/generated/common'
import { type IRestoreClient, RestoreClient } from '@/generated/restore.client'
import type { RestoreSession } from '@/generated/restore'

export const useRestoreClientStore = defineStore('config', () => {
  const transport = ref(new GrpcWebFetchTransport({ baseUrl: 'http://127.0.0.1:9090' }))

  const configClient = ref<IConfigClient | null>(null)
  const catalogs = ref<Catalog[]>([])

  const databaseClient = ref<IDatabaseClient | null>(null)

  const restoreClient = ref<IRestoreClient | null>(null)

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
      return
    }

    const call = databaseClient.value?.listClients({ catalog: catalog.id })
    for await (const client of call?.responses!) {

    }
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
    let sessions: RestoreSession[] = []
    try {
      const response = await restoreClient.value?.listSessions({})
      sessions = response?.response.sessions!
    } finally {
      return sessions
    }
  }

  return {
    transport,
    catalogs,
    fetchCatalogs,
    fetchClients,
    fetchJobs,
    fetchSessions
  }
})