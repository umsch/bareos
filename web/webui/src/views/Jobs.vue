<template>
  <section>
    <job-listing v-on:job-selected="selectedJob = $event"/>
    <job-details :jobid="selectedJob"></job-details>
    <job-log :jobid="selectedJob"></job-log>
  </section>
</template>

<script>
import JobListing from '../components/JobListing'
import JobDetails from '../components/JobDetails'
import JobLog from '../components/JobLog'

export default {
  name: 'Jobs',
  data () {
    return {
      selectedJob: 0
    }
  },
  components: {
    JobLog,
    JobDetails,
    JobListing
  },
  methods: {}
}
</script>

<style scoped>

</style>
