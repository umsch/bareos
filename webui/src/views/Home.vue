<template>
  <div class="home">
    <HelloWorld/>
    <Jobs/>
  </div>
</template>

<script>
// @ is an alias to /src
import HelloWorld from '@/components/HelloWorld.vue'
import Jobs from '@/components/Jobs.vue'

export default {
  name: 'home',
  components: {
    HelloWorld,
    Jobs
  }
}
</script>
