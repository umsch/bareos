import { useGrpcStore } from 'stores/grpcStore'

import { Catalog, type CatalogId } from 'src/generated/config'
import { Client, Job, JobType } from 'src/generated/database'
import { IDatabaseClient } from 'src/generated/database.client'

const all = { offset: 0, limit: 100 }

export class Database {
  private readonly databaseClient: IDatabaseClient

  public constructor() {
    const grpcStore = useGrpcStore()
    this.databaseClient = grpcStore.databaseClient!
  }

  async listClients(catalog: Catalog): Promise<Client[]> {
    const result = await this.databaseClient.listClients({
      catalog: catalog.id,
      filters: [],
      options: { range: all },
    })
    return result.response.clients ?? []
  }

  async listJobs(catalog_id: CatalogId, client: Client): Promise<Job[]> {
    console.debug(client)
    const response = await this.databaseClient.listJobs({
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
}
