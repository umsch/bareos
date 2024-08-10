import { createApp } from 'vue'
import { createPinia } from 'pinia'
import App from './App.vue'
import router from './router'

import Oruga from '@oruga-ui/oruga-next'

import { bulmaConfig } from '@oruga-ui/theme-bulma'
import '@oruga-ui/theme-bulma/dist/bulma.css'

const app = createApp(App)
  .use(Oruga, bulmaConfig)
  .use(createPinia())
  .use(router)

app.mount('#app')
