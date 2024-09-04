import { defineStore } from 'pinia'
import { ref, shallowRef, watch } from 'vue'
import { GrpcWebFetchTransport } from '@protobuf-ts/grpcweb-transport'
import { ConfigClient, IConfigClient } from 'src/generated/config.client'
import { DatabaseClient, IDatabaseClient } from 'src/generated/database.client'

export const useGrpcStore = defineStore('transportStore', () => {
  // todo: make url configurable
  const baseUrl = ref('http://127.0.0.1:9090')
  const transport = shallowRef<GrpcWebFetchTransport | null>(null)
  const configClient = shallowRef<IConfigClient | null>(null)
  const databaseClient = shallowRef<IDatabaseClient | null>(null)

  watch(
    baseUrl,
    (newBaseUrl) => {
      // Nur neu erzeugen, wenn baseUrl geändert wird
      transport.value = new GrpcWebFetchTransport({
        baseUrl: newBaseUrl,
        format: 'binary',
      })

      configClient.value = new ConfigClient(transport.value)
      databaseClient.value = new DatabaseClient(transport.value)
    },
    { immediate: true }
  ) // immediate ensures the watch callback runs immediately, initializing transport and configClient

  return { baseUrl, configClient, databaseClient }
})
