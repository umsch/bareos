<script setup lang="ts">
import CatalogSelector from 'components/restore/CatalogSelector.vue'
import JobSelector from 'components/restore/JobSelector.vue'
import ClientSelector from 'components/restore/ClientSelector.vue'
import FilesBrowser from 'components/restore/FilesBrowser.vue'
</script>

<template>
  <div class="row">
    <div>
      <q-card class="q-ma-sm">
        <q-card-section>
          <catalog-selector />
        </q-card-section>
        <q-card-section>
          <job-selector />
        </q-card-section>
        <q-card-section>
          <client-selector />
        </q-card-section>
      </q-card>
    </div>
    <div class="col-grow q-ma-sm">
      <q-card>
        <q-card-section>
          <files-browser />
        </q-card-section>
      </q-card>
    </div>
  </div>
</template>

<style scoped></style>
