<template>
  <div class="container">
    <p>BConsole</p>
    <vue-static-terminal></vue-static-terminal>
  </div>
</template>

<script>
import VueStaticTerminal from 'vue-static-terminal'

export default {
  name: 'BConsole',
  components: { VueStaticTerminal }
}
</script>

<style scoped>
  @import '~vue-static-terminal/dist/vue-static-terminal.css';

</style>
