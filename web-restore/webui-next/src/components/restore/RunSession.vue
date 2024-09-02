<script setup lang="ts">
import { matRocketLaunch } from '@quasar/extras/material-icons'
import { storeToRefs } from 'pinia'

import { useWizardStore } from 'stores/wizardStore'
import { ref, watch } from 'vue'
const wizardStore = useWizardStore()

const { sessionState } = storeToRefs(wizardStore)

const canRun = ref(false)

watch([() => sessionState.value?.filesMarkedCount], (markedFiles) => {
  canRun.value = Number(markedFiles) !== 0
})
</script>

<template>
  <q-btn
    :icon="matRocketLaunch"
    label="Start"
    color="primary"
    :disabled="!canRun"
  />
</template>

<style scoped></style>
