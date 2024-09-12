<script setup lang="ts">
import { matRocketLaunch } from '@quasar/extras/material-icons'

import { storeToRefs } from 'pinia'

import { useWizardStore } from 'stores/wizardStore'

const { sessionState } = storeToRefs(useWizardStore())
</script>

<template>
  <q-field
    v-model="sessionState"
    borderless
    dense
    :rules="[
      (val) =>
        (!!val && val.filesMarkedCount > 0) || 'Dateien zum restore auswählen',
    ]"
  >
    <q-btn
      type="submit"
      :icon="matRocketLaunch"
      label="Start"
      color="primary"
    />
  </q-field>
</template>

<style scoped></style>
