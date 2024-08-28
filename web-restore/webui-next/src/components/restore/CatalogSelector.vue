<script setup lang="ts">
import { computed, onBeforeMount, ref, watch } from 'vue';
import { useWizardStore } from 'src/stores/wizardStore';

import type { Catalog } from 'src/generated/config';

const wizard = useWizardStore();

const searchString = ref<string>();
const filteredData = computed(() =>
  wizard.catalogs.filter(
    (option) =>
      option.name
        .toString()
        .toLowerCase()
        .indexOf(String(searchString.value ?? '').toLowerCase()) >= 0
  )
);

// watch on selectedCatalog is used for initial catalog selection
watch(
  () => wizard.selectedCatalog,
  (selected) => {
    searchString.value = selected?.name;
  }
);

onBeforeMount(() => {
  searchString.value = wizard.selectedCatalog?.name;
});
</script>
<template>
  <o-field label="Catalog:">
    <o-autocomplete
      v-model="searchString"
      field="name"
      placeholder="Select a catalog"
      expanded
      clearable
      open-on-focus
      keep-first
      :data="filteredData"
      @select="(option: Catalog) => (wizard.selectedCatalog = option)"
    >
      <template #empty>No catalogs found</template>
    </o-autocomplete>
  </o-field>
</template>
