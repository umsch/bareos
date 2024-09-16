<script setup lang="ts">
import { storeToRefs } from 'pinia'

import { QSelect } from 'quasar'

import EmptyList from 'components/EmptyList.vue'

import { useWizardStore } from 'src/stores/wizardStore'

const wizardStore = useWizardStore()
const { jobs, selectedJob, selectedCatalog, clients, selectedSourceClient } =
  storeToRefs(wizardStore)
</script>

<template>
  <div class="row q-gutter-x-md">
    <q-select
      class="col"
      filled
      :disable="!selectedCatalog"
      v-model="selectedSourceClient"
      clearable
      use-input
      hide-selected
      fill-input
      input-debounce="0"
      label="Quell Client"
      stack-label
      :options="clients"
      option-label="name"
      :rules="[(val) => !!val || 'Quell Client wählen um Job wählen zu können']"
    >
      <template v-slot:no-option>
        <empty-list />
      </template>
    </q-select>

    <q-select
      class="col"
      filled
      :disable="!selectedCatalog"
      v-model="selectedJob"
      clearable
      use-input
      hide-selected
      fill-input
      input-debounce="0"
      label="Backup Job"
      :options="jobs"
      option-label="name"
      stack-label
      :rules="[(val) => !!val || 'Job wählen']"
    >
      <template v-slot:no-option>
        <empty-list />
      </template>
    </q-select>
  </div>
</template>

<style scoped></style>
