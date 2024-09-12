<template>
  <pre>{{ config }}</pre>
</template>

<script setup lang="ts">
import { ref } from 'vue'

import { useQuasar } from 'quasar'

const $q = useQuasar()

const config = ref($q)
</script>
