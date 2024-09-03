<script setup lang="ts">
// optional ReplaceType replace = 1;
// optional bareos.config.JobId restore_job = 3;
// optional string restore_location = 4;
// optional bareos.common.Client restore_client = 5;

import { ref, watch } from 'vue'
import { ReplaceType } from 'src/generated/restore'
import { storeToRefs } from 'pinia'
import { useWizardStore } from 'stores/wizardStore'
import { QSelect } from 'quasar'

const replace = ref(ReplaceType.NEVER)
const replaceOptions = ref([
  { label: 'Immer', value: ReplaceType.ALWAYS },
  { label: 'Nie', value: ReplaceType.NEVER },
  { label: 'Wenn Neuer', value: ReplaceType.IF_NEWER },
  { label: 'Wenn Älter', value: ReplaceType.IF_OLDER },
])

const restoreLocation = ref('/tmp')

const wizard = useWizardStore()

const { clients, selectedClient } = storeToRefs(wizard)

watch(replace, (m) => {
  console.log(m)
})
</script>

<template>
  <div class="q-gutter-y-md full-width">
    <q-field
      label="Dateien bei Rücksicherung ersetzen"
      filled
      borderless
      stack-label
    >
      <template v-slot:control>
        <div class="q-mt-sm full-width text-center">
          <q-btn-toggle
            v-model="replace"
            :options="replaceOptions"
            toggle-color="primary"
          />
        </div>
      </template>
    </q-field>
    <q-select
      filled
      v-model="selectedClient"
      clearable
      use-input
      hide-selected
      fill-input
      input-debounce="0"
      label="Client für Rücksicherung"
      stack-label
      :options="clients"
      option-label="name"
    >
      <template v-slot:no-option>
        <q-item>
          <q-item-section class="text-grey"> No results</q-item-section>
        </q-item>
      </template>
    </q-select>
    <q-input
      v-model="restoreLocation"
      label="Prefix Pfad für Rücksicherung auf dem Client"
      filled
      borderless
      stack-label
    >
    </q-input>
  </div>
</template>

<style scoped></style>
