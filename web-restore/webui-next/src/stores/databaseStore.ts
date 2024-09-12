import { defineStore, storeToRefs } from 'pinia'

import { useGrpcStore } from 'stores/grpcStore'

import { Catalog, type CatalogId } from 'src/generated/config'
import { Client, Job, JobType } from 'src/generated/database'

const all = { offset: BigInt(0), limit: BigInt(100) }

export const useDatabaseStore = defineStore('databseStore', () => {
  const { databaseClient } = storeToRefs(useGrpcStore())

  const listClients = async (catalog: Catalog): Promise<Client[]> => {
    const result = await databaseClient.value!.listClients({
      catalog: catalog.id,
      filters: [],
      options: { range: all },
    })
    return result.response.clients ?? []
  }

  const listJobs = async (
    catalog_id: CatalogId,
    client: Client,
  ): Promise<Job[]> => {
    console.debug(client)
    const response = await databaseClient.value?.listJobs({
      catalog: catalog_id,
      options: { range: all },
      filters: [
        {
          filterType: {
            oneofKind: 'client',
            client: { id: client.id },
          },
        },
        {
          filterType: {
            oneofKind: 'type',
            type: { type: JobType.BACKUP },
          },
        },
      ],
    })
    return response?.response.jobs ?? []
  }

  return { listClients, listJobs }
})
