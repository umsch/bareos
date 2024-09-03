<script setup lang="ts">
import { useWizardStore } from 'stores/wizardStore'
import { ref, watch } from 'vue'

const wizardStore = useWizardStore()

const mergeFilesets = ref(false)
const mergeJobs = ref(false)

watch(mergeFilesets, (val) => {
  console.debug(val)
  wizardStore
})
</script>

<template>
  <q-list>
    <q-item tag="label" v-ripple>
      <q-item-section>
        <q-item-label>Filesets zusammenühren</q-item-label>
        <q-item-label caption>
          Alle Client-Filesets werden zusammengeführt
        </q-item-label>
      </q-item-section>
      <q-item-section avatar>
        <q-toggle left-label v-model="mergeFilesets" />
      </q-item-section>
    </q-item>
    <q-item tag="label" v-ripple>
      <q-item-section>
        <q-item-label>Jobs zusammenführen</q-item-label>
        <q-item-label caption>
          Alle Jobs bis zur letzten Vollsicherung zusammenführen
        </q-item-label>
      </q-item-section>
      <q-item-section avatar>
        <q-toggle left-label v-model="mergeJobs" />
      </q-item-section>
    </q-item>
  </q-list>
</template>

<style scoped></style>
