import { RouteRecordRaw } from 'vue-router'

const routes: RouteRecordRaw[] = [
  {
    path: '/',
    name: 'start',
    component: () => import('pages/IndexPage.vue'),
  },
  {
    path: '/restore',
    name: 'restore',
    component: () => import('pages/RestorePage.vue'),
  },

  // Always leave this as last one,
  // but you can also remove it
  {
    path: '/:catchAll(.*)*',
    component: () => import('pages/ErrorNotFound.vue'),
  },
]

export default routes
