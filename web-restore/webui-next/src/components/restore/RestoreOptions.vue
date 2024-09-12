<script setup lang="ts">
import { ref, watch } from 'vue'

import { storeToRefs } from 'pinia'

import { QSelect } from 'quasar'

import { useWizardStore } from 'stores/wizardStore'

import { type Client } from 'src/generated/config'
import { ReplaceType } from 'src/generated/restore'

const wizard = useWizardStore()

const { sessionState, configClients } = storeToRefs(wizard)

const replaceType = ref(
  sessionState.value?.restoreOptions?.replace ?? ReplaceType.NEVER,
)
const replaceOptions = ref([
  { label: 'Immer', value: ReplaceType.ALWAYS },
  { label: 'Nie', value: ReplaceType.NEVER },
  { label: 'Wenn Neuer', value: ReplaceType.IF_NEWER },
  { label: 'Wenn Älter', value: ReplaceType.IF_OLDER },
])

const restoreLocation = ref('/tmp')

const restoreClient = ref()

watch(
  restoreClient,
  (rc) => {
    if (sessionState.value) {
      sessionState.value.restoreOptions = {
        ...sessionState.value.restoreOptions,
        restoreClient: rc?.id ?? null,
      }
    }
  },
  {
    immediate: false,
  },
)

watch(
  replaceType,
  (replaceType) => {
    if (sessionState.value) {
      sessionState.value.restoreOptions = {
        ...sessionState.value.restoreOptions,
        replace: replaceType,
      }
    }
  },
  {
    immediate: false,
  },
)

watch(
  sessionState,
  (ss) => {
    console.debug(
      'sessions state restore options',
      sessionState.value?.restoreOptions,
    )

    replaceType.value = ss?.restoreOptions?.replace ?? ReplaceType.NEVER
    restoreLocation.value =
      ss?.restoreOptions?.restoreLocation ?? '/tmp/restore'
  },
  {
    immediate: false,
  },
)
</script>

<template>
  <div class="q-gutter-y-md full-width">
    <q-field
      :disable="!sessionState?.restoreOptions"
      label="Dateien bei Rücksicherung ersetzen"
      filled
      borderless
      stack-label
      v-model="restoreClient"
    >
      <template v-slot:control>
        <div class="q-mt-sm full-width text-center">
          <q-btn-toggle
            v-model="replaceType"
            :options="replaceOptions"
            toggle-color="primary"
          />
        </div>
      </template>
    </q-field>
    <q-select
      filled
      :disable="!sessionState?.restoreOptions"
      v-model="restoreClient"
      clearable
      use-input
      hide-selected
      fill-input
      input-debounce="0"
      label="Client für Rücksicherung"
      stack-label
      :options="configClients"
      option-label="name"
      :rules="[(value: Client) => !!value || 'Ziel Client auswählen']"
    >
      <template v-slot:no-option>
        <q-item>
          <q-item-section class="text-grey"> No results</q-item-section>
        </q-item>
      </template>
    </q-select>
    <q-input
      v-model="restoreLocation"
      :disable="!sessionState?.restoreOptions"
      label="Prefix Pfad für Rücksicherung auf dem Client"
      filled
      borderless
      stack-label
      :rules="[(val: string) =>
    (!!val && val.length > 0) || 'Prefix für Rücksicherungspfad eingeben']"
    />
  </div>
</template>

<style scoped></style>
