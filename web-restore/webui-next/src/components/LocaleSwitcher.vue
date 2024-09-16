<template>
  <q-btn color="secondary" :label="$t(locale)">
    <q-menu auto-close>
      <q-list style="min-width: 100px">
        <q-item
          v-for="language in Object.getOwnPropertyNames(languages)"
          :key="language"
          @click="changeLanguage(language)"
          clickable
        >
          <q-item-section>{{ $t(language) }}</q-item-section>
        </q-item>
      </q-list>
    </q-menu>
  </q-btn>
</template>

<script setup>
import { storeToRefs } from 'pinia'

import { useLocaleStore } from 'stores/localeStore'

import languages from 'src/i18n'

const { locale } = storeToRefs(useLocaleStore())

function changeLanguage(newLanguage) {
  console.log('Changing language to', newLanguage)
  locale.value = newLanguage
}
</script>
