import { defineStore } from 'pinia'
import { ref, shallowRef, watch } from 'vue'
import { GrpcWebFetchTransport } from '@protobuf-ts/grpcweb-transport'
import { ConfigClient, IConfigClient } from 'src/generated/config.client'
import { DatabaseClient, IDatabaseClient } from 'src/generated/database.client'
import { IRestoreClient, RestoreClient } from 'src/generated/restore.client'
import { ServerStreamingCall, UnaryCall } from '@protobuf-ts/runtime-rpc'
import type { MethodInfo } from '@protobuf-ts/runtime-rpc/build/types/reflection-info'
import type { RpcOptions } from '@protobuf-ts/runtime-rpc/build/types/rpc-options'
import {
  NextUnaryFn,
  NextServerStreamingFn,
} from '@protobuf-ts/runtime-rpc/build/types/rpc-interceptor'
import { useQuasar } from 'quasar'

export const useGrpcStore = defineStore('transportStore', () => {
  // todo: make url configurable
  const baseUrl = ref('http://127.0.0.1:9090')
  const transport = shallowRef<GrpcWebFetchTransport | null>(null)
  const configClient = shallowRef<IConfigClient | null>(null)
  const databaseClient = shallowRef<IDatabaseClient | null>(null)
  const restoreClient = shallowRef<IRestoreClient | null>(null)

  const $q = useQuasar()

  const interceptErrorsServerStreaming = (
    next: NextServerStreamingFn,
    method: MethodInfo,
    input: object,
    options: RpcOptions
  ): ServerStreamingCall => {
    console.debug(`📞 Calling Director: ${method.name}`, JSON.stringify(input))
    const call = next(method, input, options)

    // Handle response stream
    call.responses.onMessage((response) => {
      console.debug(
        `👂  🐜🐜🐜 Received response from Director ${method.name}:`,
        JSON.stringify(response)
      )
    })

    // Handle errors in the stream
    call.responses.onError((error) => {
      console.error(
        `😳 Error in Director streaming call for ${method.name}:`,
        error
      )
      $q.notify({
        color: 'negative',
        caption: error.message,
        multiLine: true,
        message: `😳 Error in Director streaming call of ${method.name}:`,
        icon: 'report_problem',
      })
    })

    // Optionally handle completion of the stream
    call.responses.onComplete(() => {
      console.debug(
        `💤💤💤 Completed Director streaming call for ${method.name}`
      )
    })

    return call
  }

  const interceptErrorsUnary = (
    next: NextUnaryFn,
    method: MethodInfo,
    input: object,
    options: RpcOptions
  ): UnaryCall => {
    console.debug(`📞 Calling Director: ${method.name}`, JSON.stringify(input))
    const call: UnaryCall = next(method, input, options)

    call.response.then(
      (result) => {
        console.debug(
          `👂 Received from Director (${method.name}) :`,
          JSON.stringify(result)
        )
      },
      (error) => {
        console.error('😳 Error from Director:', method, JSON.stringify(error))
        $q.notify({
          color: 'negative',
          multiLine: true,
          caption: error.message,
          message: `😳 Error from Director when calling ${method.name}:`,
          icon: 'report_problem',
        })
      }
    )

    return call
  }

  watch(
    baseUrl,
    (newBaseUrl) => {
      // Nur neu erzeugen, wenn baseUrl geändert wird
      transport.value = new GrpcWebFetchTransport({
        baseUrl: newBaseUrl,
        format: 'binary',
        interceptors: [
          {
            interceptUnary: interceptErrorsUnary,
            interceptServerStreaming: interceptErrorsServerStreaming,
          },
        ],
      })

      configClient.value = new ConfigClient(transport.value)
      databaseClient.value = new DatabaseClient(transport.value)
      restoreClient.value = new RestoreClient(transport.value)
    },
    { immediate: true }
  ) // immediate ensures the watch callback runs immediately, initializing transport and configClient

  return { baseUrl, configClient, databaseClient, restoreClient }
})
