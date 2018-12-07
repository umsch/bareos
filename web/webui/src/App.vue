<template>
  <div id="app">
    <nav class="navbar is-white" role="navigation" aria-label="main navigation">
      <div class="container">
        <div class="navbar-brand">
          <router-link class="navbar-item" to="/">
            <img src="./assets/bareos-logo-small.png" width="48" height="45">
          </router-link>
        </div>
        <div class="navbar-menu is-active">
          <div class="navbar-start">
            <router-link class="navbar-item" to="/jobs"> Jobs</router-link>
          </div>
        </div>
      </div>
    </nav>
    <div class="container">
      <router-view/>
    </div>
  </div>
</template>

<style lang="scss">
  @import "~bulma";
  @import "~buefy/src/scss/buefy";

  html, body {
    font-family: 'Open Sans', serif;
    font-size: 16px;
    line-height: 1.5;
    height: 100%;
    background: #ECF0F3;
  }

  nav.navbar {
    margin-bottom: 1rem;
  }
</style>

<script>
import { Snackbar } from 'buefy/dist/components/snackbar'
import { messages } from './lib/http-client'

messages.subscribe(mes => Snackbar.open({
  duration: 5000,
  message: mes,
  type: 'is-danger',
  queue: false,
  indefinite: true
}))

export default {}

</script>
