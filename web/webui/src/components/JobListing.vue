<template>
  <b-table :data="jobs"
           :bordered="false"
           :striped="true"
           :narrowed="false"
           :hoverable="true"
           :loading="false"
           :focusable="false"
           :mobile-cards="false">
    <template slot-scope="props">
      <b-table-column field="id" label="ID" width="40" numeric>
        {{ props.row.jobid }}
      </b-table-column>
      <b-table-column field="jobstatus" label="Status" sortable centered>
        <font-awesome-icon icon="coffee" v-if="props.row.jobstatus === 'T'"/>
        <font-awesome-icon icon="walking" v-if="props.row.jobstatus === 'R'"/>
        <font-awesome-icon icon="exclamation-triangle" v-if="props.row.jobstatus === 'E'"/>
        {{ js(props.row.jobstatus) }}
      </b-table-column>
      <b-table-column field="name" label="Name" sortable>
        {{ props.row.name }}
      </b-table-column>
      <b-table-column field="level" label="Level" sortable centered>
        {{ jl(props.row.level) }}
      </b-table-column>
      <b-table-column field="type" label="Type" sortable centered>
        {{ jt(props.row.type) }}
      </b-table-column>
      <b-table-column field="starttime" label="Started at" sortable centered>
        {{ dateFormat(props.row.starttime) }}
      </b-table-column>
      <b-table-column field="endtime" label="Ended at" sortable centered>
        {{ dateFormat(props.row.endtime) }}
      </b-table-column>
      <!--"job": "backup-bareos-fd.2018-10-01_15.50.49_04",-->
      <!--"purgedfiles": "0",-->
      <!--"level": "F",-->
      <!--"clientid": "1",-->
      <!--"client": "localhost-fd",-->
      <!--"schedtime": "2018-10-01 15:50:38",-->
      <!--"realendtime": "2018-10-01 15:50:52",-->
      <!--"jobtdate": "1538401852",-->
      <!--"volsessionid": "1",-->
      <!--"volsessiontime": "1538387416",-->
      <!--"jobfiles": "201",-->
      <!--"jobbytes": "8103105",-->
      <!--"joberrors": "0",-->
      <!--"jobmissingfiles": "0",-->
      <!--"poolid": "3",-->
      <!--"poolname": "Full",-->
      <!--"priorjobid": "0",-->
      <!--"filesetid": "1",-->
      <!--"fileset": "SelfTest"-->

    </template>
  </b-table>
</template>

<script>
import { getJobs, jobLevels, jobStatus, jobTypes } from '@/models/jobs'
import * as moment from 'moment'

export default {
  name: 'JobListing',
  data () {
    return {
      jobs: ['nix']
    }
  },
  computed: {},
  methods: {
    js: function (code) {
      return jobStatus.get(code)
    },
    jt: function (code) {
      return jobTypes.get(code)
    },
    jl: function (code) {
      return jobLevels.get(code)
    },
    dateFormat: function (date) {
      return moment(date).fromNow()
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
