<script setup lang="ts">
import { computed, onBeforeMount, ref } from 'vue';
import { useWizardStore } from '@/stores/wizardStore';

import { OField } from '@oruga-ui/oruga-next';
import { isEmpty } from 'lodash';
import type { Catalog } from '@/generated/config';

const wizard = useWizardStore();

const noClients = computed(() => isEmpty(wizard.clients));
const noClientSelected = computed(() => !wizard.selectedClient);

const searchString = ref<string>();
const filteredData = computed(() =>
  wizard.clients.filter(
    (option) =>
      option.name
        .toString()
        .toLowerCase()
        .indexOf(String(searchString.value ?? '').toLowerCase()) >= 0
  )
);

onBeforeMount(() => {
  searchString.value = wizard.selectedClient?.name;
});
</script>
<template>
  <o-field
    label="Target Client"
    :variant="!noClients ? 'primary' : 'warning'"
    :message="!noClients ? undefined : 'select catalog first'"
  >
    <o-autocomplete
      v-model="searchString"
      field="name"
      placeholder="Select a client"
      expanded
      clearable
      open-on-focus
      keep-first
      :data="filteredData"
      @select="(option: Catalog) => (wizard.selectedClient = option)"
    >
      <template #empty>No catalogs found</template>
    </o-autocomplete>
  </o-field>
</template>
