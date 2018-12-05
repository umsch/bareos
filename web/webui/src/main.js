import Vue from 'vue'
import App from './App.vue'
import router from './router'
import store from './store'
import './registerServiceWorker'

import { Field, Input } from 'buefy/dist/components'
import 'buefy/dist/buefy.css'

Vue.use(Field)
Vue.use(Input)

Vue.config.productionTip = false

new Vue({
  router,
  store,
  render: h => h(App)
}).$mount('#app')
