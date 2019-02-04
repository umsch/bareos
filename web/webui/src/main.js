import Vue from 'vue'
import App from './App.vue'
import router from './router'
import store from './store'
import './registerServiceWorker'
import { apiBaseUrl } from '@/config'

import httpClient from './lib/http-client'
import VueAxios from 'vue-axios'

import Buefy from 'buefy'
import 'buefy/dist/buefy.css'

import { library } from '@fortawesome/fontawesome-svg-core'
import {
  faCoffee,
  faWalking,
  faExclamationTriangle,
  faAngleRight,
  faAngleLeft,
  faArrowUp
} from '@fortawesome/free-solid-svg-icons'

import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'
// the library
import fontawesome from '@fortawesome/fontawesome'
// add more icon categories as you want them, even works with pro packs
import brands from '@fortawesome/fontawesome-free-brands'

// asociate it to the library, if you need to add more you can separate them by a comma
fontawesome.library.add(brands)

library.add(faCoffee)
library.add(faWalking)
library.add(faAngleRight)
library.add(faAngleLeft)
library.add(faArrowUp)
library.add(faExclamationTriangle)

Vue.component('font-awesome-icon', FontAwesomeIcon)
Vue.use(Buefy, {
  materialDesignIcons: false,
  defaultIconPack: 'fas'
})

Vue.use(VueAxios, httpClient({
  baseUrl: apiBaseUrl
}))

import VueStaticTerminal from 'vue-static-terminal'

Vue.use(VueStaticTerminal)

Vue.config.productionTip = false

new Vue({
  router,
  store,
  render: h => h(App)
}).$mount('#app')
