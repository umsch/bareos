import { defineStore, storeToRefs } from 'pinia'
import type { Catalog, Client, Job } from 'src/generated/config'
import { useGrpcStore } from 'stores/grpcStore'
import { JobType } from 'src/generated/common'

export const useConfigStore = defineStore('configStore', () => {
  const { configClient } = storeToRefs(useGrpcStore())

  const getConfigClients = async (): Promise<Client[]> => {
    const result = await configClient.value!.listClients({})
    return result.response.clients
  }

  const getConfigJobs = async (jobType: JobType): Promise<Job[]> => {
    const result = await configClient.value!.listJobs({
      filters: [
        {
          filterType: {
            oneofKind: 'type',
            type: {
              select: jobType,
            },
          },
        },
      ],
    })
    return result.response.jobs
  }

  const getConfigCatalogs = async (): Promise<Catalog[]> => {
    const result = await configClient.value!.listCatalogs({})
    return result.response.catalogs
  }

  return { getConfigCatalogs, getConfigClients, getConfigJobs }
})
