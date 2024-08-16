<script setup lang="ts">
import { computed } from 'vue'
import { useWizardStore } from '@/stores/wizardStore'

import { OField } from '@oruga-ui/oruga-next'
import { isEmpty } from 'lodash'

const wizardStore = useWizardStore()

const noClients = computed(() => isEmpty(wizardStore.clients))
const noClientSelected = computed(() => !wizardStore.selectedClient)
</script>
<template>
  <o-field
    :variant="!noClients ? 'primary' : 'warning'"
    :message="!noClients ? undefined : 'select catalog first'"
  >
    <o-dropdown v-model="wizardStore.selectedClient">
      <template #trigger="{ active }">
        <o-button
          variant="inverted"
          :label="!noClientSelected ? wizardStore.selectedClient?.name : 'Select Client'"
          :icon-right="active ? 'caret-up' : 'caret-down'"
        />
      </template>

      <o-dropdown-item v-for="(client, index) in wizardStore.clients" :key="index" :value="client">
        <div>
          <div>{{ client.name }}</div>
        </div>
      </o-dropdown-item>
    </o-dropdown>
  </o-field>
</template>
