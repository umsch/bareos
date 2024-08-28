<script setup lang="ts">
import { computed, onBeforeMount, onMounted, ref, watch } from 'vue';

import { useWizardStore } from '@/stores/wizardStore';
import type { Job } from '@/generated/common';

const wizard = useWizardStore();

const searchString = ref<string>();
const filteredData = computed(() =>
  wizard.jobs.filter(
    (option) =>
      option.jobid
        .toString()
        .toLowerCase()
        .indexOf(String(searchString?.value ?? '').toLowerCase()) >= 0
  )
);

onBeforeMount(() => {
  searchString.value = wizard.selectedJob?.jobid.toString();
});
</script>

<template>
  <o-field label="Backup Job">
    <o-autocomplete
      v-model="searchString"
      field="jobid"
      placeholder="Select a job"
      expanded
      clearable
      open-on-focus
      keep-first
      :data="filteredData"
      @select="(option: Job) =>  (wizard.selectedJob = option)"
    >
      <template #empty>No jobs found</template>
    </o-autocomplete>
  </o-field>
</template>

<style scoped></style>
