<script setup lang="ts">

import { ref } from 'vue'
import { OIcon } from '@oruga-ui/oruga-next'

const activeStep = ref('1')
const prevIcon = ref('chevron-left')
const nextIcon = ref('chevron-right')

</script>

<template>
  <section>
    <o-steps
      variant="primary"
      :v-model="activeStep"
      animated
      rounded
      has-navigation
      :icon-prev="prevIcon"
      :icon-next="nextIcon"
    >
      <o-step-item
        value="1"
        step="1"
        label="eins"
        clickable>
        <h1>Account</h1>
        Lorem ipsum dolor sit amet.
      </o-step-item>
    </o-steps>
  </section>


</template>

<style scoped>

</style>