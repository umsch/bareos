<template>
  <b-table class="is-fullwidth" :data="clientsStatus" :columns="columns"
           :bordered="false"
           :striped="true"
           :narrowed="false"
           :hoverable="true"
           :loading="false"
           :focusable="false"
           :mobile-cards="false"></b-table>
</template>

<script>
export default {
  name: 'HelloWorld',

  data () {
    return {
      clientsStatus: [],
      columns: [
        {
          field: 'clientid',
          label: 'ID',
          width: '40',
          numeric: true
        },
        {
          field: 'name',
          label: 'Name',
          width: '100%'
        },
        {
          field: 'fileretention',
          label: 'fileretention',
          numeric: true
        },
        {
          field: 'jobretention',
          label: 'jobretention',
          numeric: true
        }
      ]
    }
  },
  methods: {
    getList: async function () {
      try {
        const response = await this.$http.get('http://localhost:3000/list/clients')
        return response.data.clients
      } catch (e) {
        console.warn(e)
      }
      return null
    }
  },
  created: async function () {
    const clients = await this.getList()
    this.clientsStatus = clients
    console.log(clients)
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped lang="scss">
</style>
