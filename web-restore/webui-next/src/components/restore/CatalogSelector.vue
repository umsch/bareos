<script setup lang="ts">
import { storeToRefs } from 'pinia'

import { QSelect } from 'quasar'

import { useWizardStore } from 'stores/wizardStore'

import { Catalog } from 'src/generated/config'

const wizardStore = useWizardStore()
const { catalogs, selectedCatalog } = storeToRefs(wizardStore)
</script>
<template>
  <q-select
    filled
    v-model="selectedCatalog"
    clearable
    use-input
    hide-selected
    fill-input
    stack-label
    label="Catalog"
    :options="catalogs"
    option-label="name"
    :rules="[(val: Catalog) => !!val]"
  >
    <template v-slot:no-option>
      <q-item>
        <q-item-section class="text-grey"> No results </q-item-section>
      </q-item>
    </template>
  </q-select>
</template>
