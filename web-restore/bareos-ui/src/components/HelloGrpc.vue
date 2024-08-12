<template>
  <div>
    <h1>{{ message }}</h1>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { RestoreClient } from '@/generated/restore.client'
import { GrpcWebFetchTransport } from '@protobuf-ts/grpcweb-transport'

const message = ref('')

const transport = ref(new GrpcWebFetchTransport({ baseUrl: 'http://127.0.0.1:9090' }))

onMounted(async () => {
  const client = new RestoreClient(transport.value)
  const response = await client.listSessions({})

  console.log('hier: ', response)
  message.value = response.response
})
</script>
