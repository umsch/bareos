<script setup lang="ts">
import CatalogSelector from 'components/restore/CatalogSelector.vue'
import JobSelector from 'components/restore/JobSelector.vue'
import ClientSelector from 'components/restore/ClientSelector.vue'
import FilesBrowser from 'components/restore/FilesBrowser.vue'
</script>

<template>
  <div class="q-pa-md row items-start q-gutter-md">
    <q-card class="col-shrink">
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
    <q-card class="col-grow">
      <q-card-section>
        <files-browser />
      </q-card-section>
    </q-card>
  </div>
</template>

<style scoped></style>
