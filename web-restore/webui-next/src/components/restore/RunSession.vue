<script setup lang="ts">
import { matRocketLaunch } from '@quasar/extras/material-icons'
import { storeToRefs } from 'pinia'

import { useWizardStore } from 'stores/wizardStore'
import { ref, watch } from 'vue'
const wizardStore = useWizardStore()

const { sessionState } = storeToRefs(wizardStore)

const canStart = ref(false)
const executeStart = () => {
  wizardStore.runRestoreSession()
}

watch([() => sessionState.value?.filesMarkedCount], (markedFiles) => {
  canStart.value = Number(markedFiles) !== 0
})
</script>

<template>
  <q-field
    borderless
    dense
    hint="hier fehlen noch hinweise darauf, welche Eingaben noch fehlen"
  >
    <q-btn
      :icon="matRocketLaunch"
      label="Start"
      color="primary"
      :disabled="!canStart"
      @click="executeStart"
    />
  </q-field>
</template>

<style scoped></style>
