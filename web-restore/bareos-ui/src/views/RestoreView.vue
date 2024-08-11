<script setup lang="ts">

import { onMounted, ref } from 'vue'

import { ConfigClient, type IConfigClient } from '@/generated/config.client'
import { GrpcWebFetchTransport } from '@protobuf-ts/grpcweb-transport'
import { ODropdown } from '@oruga-ui/oruga-next'
import type { Catalog } from '@/generated/config'

const activeStep = ref('1')
const prevIcon = ref('chevron-left')
const nextIcon = ref('chevron-right')

const transport = ref(new GrpcWebFetchTransport({ baseUrl: 'http://127.0.0.1:9090' }))
const configClient = ref<IConfigClient>()
const catalogs = ref<Catalog[]>([])
const selectedCatalog = ref<Catalog>()

const fetchCatalogs = async () => {
  if (!configClient.value) {
    console.error('configClient not initialized')
    return
  }

  const response = await configClient.value.listCatalogs({})
  catalogs.value = response.response.catalogs
}

onMounted(() => {
  configClient.value = new ConfigClient(transport.value)
  fetchCatalogs()
})


</script>

<template>
  <section>
    <o-field label="Catalog">
      <o-dropdown v-model="selectedCatalog">
        <template #trigger="{ active }">
          <o-button
            variant="primary"
            :label="selectedCatalog ? selectedCatalog.name : 'Select Catalog'"
            :icon-right="active ? 'caret-up' : 'caret-down'" />
        </template>

        <o-dropdown-item
          v-for="(catalog, index) in catalogs"
          :key="index"
          :value="catalog">
          <div>
            <div>{{ catalog.name }}</div>
          </div>
        </o-dropdown-item>

      </o-dropdown>
    </o-field>


    <o-steps
      variant="primary"
      :v-model="activeStep"
      animated
      rounded
      has-navigation
      :icon-prev="prevIcon"
      :icon-next="nextIcon"
    >
      <o-step-item
        value="1"
        step="1"
        label="eins"
        clickable>
        <h1>Account</h1>
        Lorem ipsum dolor sit amet.
      </o-step-item>
    </o-steps>
  </section>


</template>

<style scoped>

</style>