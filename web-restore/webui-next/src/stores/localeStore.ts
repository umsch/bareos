import { useI18n } from 'vue-i18n'

import { ref, watch } from 'vue'

import { defineStore } from 'pinia'

export const useLocaleStore = defineStore(
  'localeStore',
  () => {
    const i18n = useI18n({ useScope: 'global' })

    const locale = ref()

    watch(locale, (lang) => {
      i18n.locale.value = lang
    })

    return {
      locale,
    }
  },
  {
    persist: {
      storage: sessionStorage,
      pick: ['locale'],
    },
  },
)
