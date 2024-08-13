<script setup lang="ts">
import { computed } from 'vue'
import { useWizardStore } from '@/stores/wizardStore'

import { OField } from '@oruga-ui/oruga-next'
import { isEmpty } from 'lodash'

const wizardStore = useWizardStore()

const noCatalogs = computed(() => isEmpty(wizardStore.catalogs))
const noCatalogSelected = computed(() => !wizardStore.selectedCatalog)

</script>
<template>
  <o-field
    label="Catalog:"
    :variant="!noCatalogs ? 'primary' : 'warning'"
    :message="!noCatalogs ? undefined : 'no catalogs found'">

    <o-dropdown v-model="wizardStore.selectedCatalog">
      <template #trigger="{ active }">
        <o-button
          variant="primary"
          :label="!noCatalogSelected ? wizardStore.selectedCatalog?.name : 'Select Catalog'"
          :icon-right="active ? 'caret-up' : 'caret-down'"
        />
      </template>

      <o-dropdown-item v-for="(catalog, index) in wizardStore.catalogs" :key="index" :value="catalog">
        <div>
          <div>{{ catalog.name }}</div>
        </div>
      </o-dropdown-item>
    </o-dropdown>
  </o-field>
</template>
