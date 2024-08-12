<script setup lang="ts">
import { defineProps, defineEmits, ref, watch, onMounted } from 'vue'
import type { Catalog } from '@/generated/config'

import { useRestoreClientStore } from '@/stores/restoreClientStore'

const emit = defineEmits<{
  (e: 'update:selectedCatalog', catalog: Catalog): void
}>()

const restoreClientStore = useRestoreClientStore()

const selected = ref<Catalog | null>(null)

onMounted(async () => {
  await updateCatalogs()
})

watch(selected, (newValue) => {
  if (newValue) {
    emit('update:selectedCatalog', newValue)
  }
})

const catalogs = ref<Catalog[]>([])
const updateCatalogs = async () => {
  catalogs.value = await restoreClientStore.fetchCatalogs()
}
</script>
<template>
  <o-field label="Catalog">
    <o-dropdown v-model="selected">
      <template #trigger="{ active }">
        <o-button
          variant="primary"
          :label="selected ? selected.name : 'Select Catalog'"
          :icon-right="active ? 'caret-up' : 'caret-down'"
        />
      </template>

      <o-dropdown-item v-for="(catalog, index) in catalogs" :key="index" :value="catalog">
        <div>
          <div>{{ catalog.name }}</div>
        </div>
      </o-dropdown-item>
    </o-dropdown>
  </o-field>
</template>
