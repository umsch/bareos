<script setup lang="ts">
import { useWizardStore } from 'src/stores/wizardStore'

import { QSelect } from 'quasar'
import { storeToRefs } from 'pinia'

const wizardStore = useWizardStore()

const { clients, selectedClient } = storeToRefs(wizardStore)
</script>
<template>
  <q-select
    filled
    v-model="selectedClient"
    clearable
    use-input
    hide-selected
    fill-input
    input-debounce="0"
    label="Client"
    :options="clients"
    option-label="name"
  >
    <template v-slot:no-option>
      <q-item>
        <q-item-section class="text-grey"> No results</q-item-section>
      </q-item>
    </template>
  </q-select>
</template>
