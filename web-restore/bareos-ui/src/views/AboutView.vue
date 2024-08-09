<template>
  <div class="about">
    <HelloGrpc />
  </div>
</template>

<style>
@media (min-width: 1024px) {
  .about {
    min-height: 100vh;
    display: flex;
    align-items: center;
  }
}
</style>
<script setup lang="ts">
import HelloGrpc from "@/components/HelloGrpc.vue";
</script>