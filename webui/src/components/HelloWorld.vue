<template>
  <div class="hello">
    <p>{{clientsStatus}}</p>
  </div>
</template>

<script>
export default {
  name: 'HelloWorld',

  data () {
    return {
      msg: 'Welcome to Your Vue.js App Bareos',
      clientsStatus: ''
    }
  },
  methods: {
    getList: async function () {
      try {
        const response = await this.$http.get('http://localhost:3000/list/clients')
        return response
      } catch (e) {
        console.warn(e)
      }
      return null
    }
  },
  created: async function () {
    this.clientsStatus = await this.getList()
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped lang="scss">
</style>
