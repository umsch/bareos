<script setup lang="ts">
import { RouterLink, RouterView } from 'vue-router'
</script>

<template>
  <header>
    <div class="wrapper">
      <nav class="navbar">
        <div class="container">
          <div class="navbar-brand"></div>
          <div class="navbar-menu">
            <div class="navbar-start">
              <a class="navbar-item">
                <RouterLink to="/">Home</RouterLink>
              </a>
              <a class="navbar-item">
                <RouterLink to="/restore">Restore</RouterLink>
              </a>
              <a class="navbar-item">
                <RouterLink to="/about">About</RouterLink>
              </a>
            </div>
          </div>
        </div>
      </nav>
    </div>
  </header>
  <RouterView />
</template>

<style lang="scss">
@import './styles/bareos';
</style>
