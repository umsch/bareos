<script setup lang="ts">
import { useWizardStore } from 'stores/wizardStore'

const wizard = useWizardStore()
</script>

<template>
  <pre class="text-black">{{ wizard.sessionState }}</pre>

  <pre class="text-black">{{ wizard.selectedSession }}</pre>
</template>

<style scoped></style>
