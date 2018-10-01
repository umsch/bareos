<template>
  <section>
    <b-table class="is-fullwidth" :data="jobs" :columns="columns"
             :bordered="false"
             :striped="true"
             :narrowed="false"
             :hoverable="true"
             :loading="false"
             :focusable="false"
             :mobile-cards="false"></b-table>
  </section>
</template>

<script>
import getJobs from '@/models/jobs'

export default {
  name: 'Jobs',
  data () {
    return {
      jobs: ['nix'],
      columns: [
        {
          field: 'jobid',
          label: 'ID',
          width: '40',
          numeric: true
        },
        {
          field: 'job',
          label: 'Job',
          width: '100%'
        },
        {
          field: 'name',
          label: 'Name'
        },
        {
          field: 'name',
          label: 'Name'
        }
      ]
    }
  },
  created: async function () {
    const jobs = await getJobs(this.$http)
    this.jobs = jobs
    console.log(jobs)
  }
}
</script>

<style scoped>

</style>
