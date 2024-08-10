import { createApp } from 'vue'
import { createPinia } from 'pinia'
import App from './App.vue'
import router from './router'

import Oruga from '@oruga-ui/oruga-next'

import { bulmaConfig } from '@oruga-ui/theme-bulma'
import '@oruga-ui/theme-bulma/dist/bulma.css'

import { library } from '@fortawesome/fontawesome-svg-core';
import { fas } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/vue-fontawesome";
library.add(fas);


const config = {
  ...bulmaConfig,
  iconComponent: "vue-fontawesome",
  iconPack: "fas",
};



const app = createApp(App)
  .use(Oruga, config)
  .use(createPinia())
  .use(router)
  .component("vue-fontawesome", FontAwesomeIcon);

app.mount('#app')
