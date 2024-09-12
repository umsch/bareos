<script setup lang="ts">
import { computed } from 'vue'

import { storeToRefs } from 'pinia'

import { useWizardStore } from 'stores/wizardStore'

const wizardStore = useWizardStore()
const { selectedCatalog, sessionState, mergeFilesets, findJobChain } =
  storeToRefs(wizardStore)

const disabled = computed(
  () => !selectedCatalog || !!sessionState.value?.restoreOptions,
)
</script>

<template>
  <q-list>
    <q-item tag="label" v-ripple :disable="disabled">
      <q-item-section>
        <q-item-label>Filesets zusammenühren</q-item-label>
        <q-item-label caption>
          Alle Client-Filesets werden zusammengeführt
        </q-item-label>
      </q-item-section>
      <q-item-section avatar>
        <q-toggle left-label v-model="findJobChain" :disable="disabled" />
      </q-item-section>
    </q-item>
    <q-item tag="label" v-ripple :disable="disabled">
      <q-item-section>
        <q-item-label>Jobs zusammenführen</q-item-label>
        <q-item-label caption>
          Alle Jobs bis zur letzten Vollsicherung zusammenführen
        </q-item-label>
      </q-item-section>
      <q-item-section avatar>
        <q-toggle left-label v-model="mergeFilesets" :disable="disabled" />
      </q-item-section>
    </q-item>
  </q-list>
</template>

<style scoped></style>
