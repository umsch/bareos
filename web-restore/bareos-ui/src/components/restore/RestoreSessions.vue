<script setup lang="ts">
import { useWizardStore } from '@/stores/wizardStore'
import { ODropdown, OField } from '@oruga-ui/oruga-next'
import { isEmpty } from 'lodash'
import { computed } from 'vue'

const wizardStore = useWizardStore()

const noSessions = computed(() => isEmpty(wizardStore.sessions))
</script>

<template>
  <o-field
    label="Session:"
    :variant="!noSessions ? 'primary' : 'warning'"
    :message="!noSessions ? undefined : 'no sessions found'"
  >
    <o-dropdown v-model="wizardStore.selectedSession" :disabled="noSessions">
      <template #trigger="{ active }">
        <o-button
          variant="primary"
          :label="
            wizardStore.selectedSession
              ? wizardStore.selectedSession.token
              : 'Select Restore Session'
          "
          :icon-right="active ? 'caret-up' : 'caret-down'"
        />
      </template>

      <o-dropdown-item
        v-for="(session, index) in wizardStore.sessions"
        :key="index"
        :value="session"
      >
        <div>
          <div>{{ session.token }}</div>
        </div>
      </o-dropdown-item>
    </o-dropdown>
  </o-field>
</template>

<style scoped></style>
