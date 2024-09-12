<script setup lang="ts">
import { ref, watch } from 'vue'

import { storeToRefs } from 'pinia'

import { QForm } from 'quasar'

import { useWizardStore } from 'stores/wizardStore'

import CatalogSelector from 'components/restore/CatalogSelector.vue'
import FilesBrowser from 'components/restore/FilesBrowser.vue'
import JobSelector from 'components/restore/JobSelector.vue'
import MergeFilesetsSwitch from 'components/restore/MergeFilesetsSwitch.vue'
import RestoreOptions from 'components/restore/RestoreOptions.vue'
import RunSession from 'components/restore/RunSession.vue'

const wizard = useWizardStore()
const { selectedCatalog, selectedSourceClient, selectedJob, sessionState } =
  storeToRefs(wizard)

const executeStart = async () => {
  await wizard.runRestoreSession()
  // wizard.$reset()
}

const form = ref<QForm>()
watch(
  [selectedCatalog, selectedSourceClient, selectedJob, sessionState],
  async () => {
    await form.value!.validate()
  },
  {
    flush: 'post',
  },
)
</script>

<template>
  <div class="q-pa-md row items-start q-gutter-x-md">
    <div class="q-gutter-y-md col-shrink">
      <q-card>
        <q-form greedy @submit="executeStart" ref="form">
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
