<template>
  <section>
    <table class="table is-striped">
      <thead>
      <th>Timestamp</th>
      <th>Log</th>
      </thead>
      <template v-for="log in jobLog">
        <tr>
          <td>{{log.time}}</td>
          <td>{{log.logtext}}</td>
        </tr>
      </template>
    </table>
  </section>
</template>

<script>
import { getJobLog } from '@/models/jobs'

export default {
  name: 'JobLog',
  props: {
    jobid: Number
  },
  data () {
    return {
      job: Object,
      jobLog: Object
    }
  },
  watch: {
    jobid: async function (val) {
      // todo: check result and catch exceptions
      this.jobLog = await getJobLog(this.$http, val)
    }
  }
}
</script>

<style scoped>

</style>
