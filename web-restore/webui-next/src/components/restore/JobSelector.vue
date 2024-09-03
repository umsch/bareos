<script setup lang="ts">
import { useWizardStore } from 'src/stores/wizardStore'
import { QSelect } from 'quasar'
import { storeToRefs } from 'pinia'

const wizardStore = useWizardStore()

const { jobs, selectedJob } = storeToRefs(wizardStore)
</script>

<template>
  <q-select
    filled
    v-model="selectedJob"
    clearable
    use-input
    hide-selected
    fill-input
    input-debounce="0"
    label="Backup Job"
    :options="jobs"
    option-label="jobid"
    stack-label
  >
    <template v-slot:no-option>
      <q-item>
        <q-item-section class="text-grey"> No results </q-item-section>
      </q-item>
    </template>
  </q-select>
</template>

<style scoped></style>
