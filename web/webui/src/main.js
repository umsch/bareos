import Vue from 'vue'
import App from './App.vue'
import router from './router'
import store from './store'
import './registerServiceWorker'

import axios from 'axios'
import VueAxios from 'vue-axios'

import { Field, Input } from 'buefy/dist/components'
import 'buefy/dist/buefy.css'

Vue.use(Field)
Vue.use(Input)
Vue.use(VueAxios, axios)

Vue.config.productionTip = false

new Vue({
  router,
  store,
  render: h => h(App)
}).$mount('#app')
