<template>
  <div>
    <section class="hero">
      <div class="hero-head">
        <nav class="navbar" role="navigation" aria-label="main navigation">
          <div class="navbar-brand">
            <!-- navbar items, navbar burger... -->
            <img src="@/assets/bareos-logo-small.png"/>
          </div>
          <div class="navbar-menu">
            <div class="navbar-item">
              <router-link to="/">Home</router-link>
            </div>
            <div class="navbar-item">
              <router-link to="/about">About</router-link>
            </div>
          </div>
        </nav>
      </div>
    </section>
    <div class="container">
      <router-view/>
    </div>
  </div>
</template>

<style lang="scss">
  @import "~bulma";
  @import "~buefy/src/scss/buefy";

  #app {
    font-family: 'Avenir', Helvetica, Arial, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;
    color: #2c3e50;
  }

  #nav {
    padding: 30px;
    a {
      font-weight: bold;
      color: #2c3e50;
      &.router-link-exact-active {
        color: #42b983;
      }
    }
  }
</style>
