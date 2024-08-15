<script setup lang="ts">
import { RouterLink, RouterView } from 'vue-router'
import { ref } from 'vue'

const isMenuOpen = ref(false)

const toggleMenu = () => {
  isMenuOpen.value = !isMenuOpen.value
}
</script>

<template>
  <header>
    <nav class="navbar">
      <div class="navbar-brand">
        <a
          role="button"
          class="navbar-burger"
          :class="{ 'is-active': isMenuOpen }"
          aria-label="menu"
          aria-expanded="false"
          @click="toggleMenu"
        >
          <span aria-hidden="true"></span>
          <span aria-hidden="true"></span>
          <span aria-hidden="true"></span>
        </a>
      </div>
      <div class="navbar-menu" :class="{ 'is-active': isMenuOpen }">
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
    </nav>
  </header>
  <RouterView />
</template>

<style lang="scss">
@import './styles/bareos';
</style>
