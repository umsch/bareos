<script setup lang="ts">
import CatalogSelector from 'components/restore/CatalogSelector.vue'
import JobSelector from 'components/restore/JobSelector.vue'
import ClientSelector from 'components/restore/ClientSelector.vue'
import FilesBrowser from 'components/restore/FilesBrowser.vue'
import SessionState from 'components/restore/SessionState.vue'
import RunSession from 'components/restore/RunSession.vue'

import { useWizardStore } from 'stores/wizardStore'
const wizard = useWizardStore()
</script>

<template>
  <div class="q-pa-md row items-start q-gutter-x-md">
    <div class="q-gutter-y-md col-shrink">
      <q-card>
        <q-card-section>
          <catalog-selector />
        </q-card-section>
        <q-card-section>
          <job-selector />
        </q-card-section>
        <q-card-section>
          <client-selector />
        </q-card-section>
        <q-card-section>
          <run-session />
        </q-card-section>
      </q-card>

      <q-card>
        <q-card-section>
          <session-state />
        </q-card-section>
      </q-card>
    </div>
    <q-card class="col-grow">
      <q-card-section>
        <files-browser />
      </q-card-section>
    </q-card>
  </div>

  <pre>{{ wizard }}</pre>
</template>

<style scoped></style>
