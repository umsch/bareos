import { defineStore } from 'pinia'
import { ref } from 'vue'
import { GrpcWebFetchTransport } from '@protobuf-ts/grpcweb-transport'
import { ConfigClient, type IConfigClient } from '@/generated/config.client'
import type { Catalog } from '@/generated/config'
import { DatabaseClient, type IDatabaseClient } from '@/generated/database.client'
import type { Client, Job } from '@/generated/common'

export const useRestoreClientStore = defineStore('config', () => {
  const transport = ref(new GrpcWebFetchTransport({ baseUrl: 'http://127.0.0.1:9090' }))

  const configClient = ref<IConfigClient | null>(null)
  const catalogs = ref<Catalog[]>([])

  const databaseClient = ref<IDatabaseClient | null>(null)
  const clients = ref<Client[]>([])


  const initializeClient = async () => {
    configClient.value = new ConfigClient(transport.value)
    databaseClient.value = new DatabaseClient(transport.value)
    await fetchCatalogs()
  }

  const fetchCatalogs = async () => {
    if (!configClient.value) {
      console.error('configClient not initialized')
      return
    }

    const response = await configClient.value.listCatalogs({})
    catalogs.value = response.response.catalogs
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

  const fetchJobs = async (catalog: Catalog) => {
    if (!configClient.value) {
      console.error('configClient not initialized')
      return []
    }

    const call = databaseClient.value?.listJobs({ catalog: catalog.id! })
    let jobs: Job[] = []
    for await (const job of call?.responses!) {
      console.log(JSON.stringify(job.job?.jobid.toString()))
      jobs.push(job.job!)
    }

    return jobs
  }

  return { transport, catalogs, initializeClient, fetchCatalogs, fetchClients, fetchJobs }
})