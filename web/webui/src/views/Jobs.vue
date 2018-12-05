<template>
  <job-listing/>
</template>

<script>
import JobListing from '../components/JobListing'

export default {
  name: 'Jobs',
  data () {
    return {
      jobs: 'mounted'
    }
  },
  components: {
    JobListing
  },
  methods: {}
}
</script>

<style scoped>

</style>
