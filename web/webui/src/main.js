import Vue from 'vue'
import App from './App.vue'
import router from './router'
import store from './store'
import './registerServiceWorker'

import axios from 'axios'
import VueAxios from 'vue-axios'

import { Field, Input, Table } from 'buefy/dist/components'
import 'buefy/dist/buefy.css'

import { library } from '@fortawesome/fontawesome-svg-core'
import { faCoffee, faWalking, faExclamationTriangle } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'

library.add(faCoffee)
library.add(faWalking)
library.add(faExclamationTriangle)

Vue.component('font-awesome-icon', FontAwesomeIcon)

Vue.use(Field)
Vue.use(Input)
Vue.use(Table)
Vue.use(VueAxios, axios)

Vue.config.productionTip = false

new Vue({
  router,
  store,
  render: h => h(App)
}).$mount('#app')
