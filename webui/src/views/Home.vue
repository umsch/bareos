<template>
  <div class="home">
    <div class="box">
      <h3 class="title is-3">Client status</h3>
      <HelloWorld/>
    </div>
    <div class="box">
      <h3 class="title is-3">Jobs</h3>
      <Jobs/>
    </div>
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
