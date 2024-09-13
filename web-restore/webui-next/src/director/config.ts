import { useGrpcStore } from 'stores/grpcStore'

import type { Catalog, Client, Job, JobType } from 'src/generated/config'
import { IConfigClient } from 'src/generated/config.client'

export class Config {
  private readonly configClient: IConfigClient

  public constructor() {
    const grpcStore = useGrpcStore()
    this.configClient = grpcStore.configClient!
  }

  async getConfigClients(): Promise<Client[]> {
    const result = await this.configClient.listClients({})
    return result.response.clients
  }

  async getConfigJobs(jobType: JobType): Promise<Job[]> {
    const result = await this.configClient.listJobs({
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

  async getConfigCatalogs(): Promise<Catalog[]> {
    const result = await this.configClient.listCatalogs({})
    return result.response.catalogs
  }
}
