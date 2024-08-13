<script setup lang="ts">
import { ref, onMounted } from 'vue'

const message = ref('Hello, gRPC!')
</script>

<template>
  <div>
    <h1>{{ message }}</h1>
  </div>
</template>

<style scoped></style>
