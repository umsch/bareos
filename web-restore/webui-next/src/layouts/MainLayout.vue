<template>
  <q-layout view="lHh Lpr lFf">
    <q-header elevated>
      <q-toolbar>
        <q-btn
          flat
          dense
          round
          icon="menu"
          aria-label="Menu"
          @click="toggleLeftDrawer"
        />

        <q-toolbar-title> Bareos - Restore Client </q-toolbar-title>

        <LocaleSwitcher />
      </q-toolbar>
    </q-header>

    <q-drawer v-model="leftDrawerOpen" show-if-above bordered>
      <q-list>
        <q-item v-for="link in linksList" :key="link.title">
          <EssentialLink v-bind="link" />
        </q-item>
      </q-list>
    </q-drawer>

    <q-page-container>
      <router-view />
    </q-page-container>
  </q-layout>
</template>

<script setup lang="ts">
import { ref } from 'vue'

import EssentialLink, { EssentialLinkProps } from 'components/EssentialLink.vue'
import LocaleSwitcher from 'components/LocaleSwitcher.vue'

defineOptions({
  name: 'MainLayout',
})

const linksList: EssentialLinkProps[] = [
  {
    title: 'main_menu.start.title',
    caption: 'main_menu.start.caption',
    icon: 'house',
    link: '/',
  },
  {
    title: 'main_menu.restore.title',
    caption: 'main_menu.restore.caption',
    icon: 'restore',
    link: '/restore',
  },
]

const leftDrawerOpen = ref(false)

function toggleLeftDrawer() {
  leftDrawerOpen.value = !leftDrawerOpen.value
}
</script>
