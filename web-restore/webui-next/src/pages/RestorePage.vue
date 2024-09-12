<script setup lang="ts">
import { useWizardStore } from 'stores/wizardStore'

import CatalogSelector from 'components/restore/CatalogSelector.vue'
import FilesBrowser from 'components/restore/FilesBrowser.vue'
import JobSelector from 'components/restore/JobSelector.vue'
import MergeFilesetsSwitch from 'components/restore/MergeFilesetsSwitch.vue'
import RestoreOptions from 'components/restore/RestoreOptions.vue'
import RunSession from 'components/restore/RunSession.vue'
import SessionState from 'components/restore/SessionState.vue'

const wizard = useWizardStore()

const executeStart = async () => {
  await wizard.runRestoreSession()
  wizard.$reset()
}
</script>

<template>
  <div class="q-pa-md row items-start q-gutter-x-md">
    <div class="q-gutter-y-md col-shrink">
      <q-card>
        <q-form greedy @submit="executeStart">
          <q-card-section>
            <catalog-selector />
          </q-card-section>
          <q-card-section>
            <merge-filesets-switch />
          </q-card-section>
          <q-card-section>
            <job-selector />
          </q-card-section>
          <q-card-section>
            <restore-options />
          </q-card-section>
          <q-card-section>
            <run-session />
          </q-card-section>
        </q-form>
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
