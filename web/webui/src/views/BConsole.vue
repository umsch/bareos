<template>
  <div class="container">
    <section>
      <pre>{{ $route.params.id }}</pre>
    </section>
    <section>
      <b-field label="Command">
        <b-input
          v-model="command"
          @keydown.native.enter="onSubmit"
        />
      </b-field>
    </section>
  </div>
</template>
<script>

export default {
  name: 'BConsole',
  data () {
    return {
      command: '',
      result: 'test\nnewline'
    }
  },
  methods: {
    onSubmit: function () {
      console.log(this.command)
      this.command = ''
    }
  }
}
</script>

<style scoped>
</style>
