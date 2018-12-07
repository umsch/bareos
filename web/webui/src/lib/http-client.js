import axios from 'axios'
import { Subject } from 'rxjs'

export const messages = new Subject()

export default function (params) {
  axios.defaults.baseURL = params.baseUrl
  axios.interceptors.response.use(value => {
    return value
  }, error => {
    console.error(`Error getting data. ${error.request.responseURL}: status: ${error.response.status}, message: ${error.response.data}`)
    messages.next(error.response.data)

    if (error.response) {
      // React on response status.
      switch (error.response.status) {
        case 401:
          if (error.config.url !== opts.baseURL + '/login') {
            console.log('[auth] JWT has expired.')
            // Redirect to login.
            opts.auth.logout()
          }
          break
        case 403:
          messages.next('[auth] No permission to perform operation.')
          break
        case 500:
          messages.next('Server error.')
          break
        case 503:
          messages.next('Service is not ready yet.')
          // opts.auth.logout()
          // opts.router.replace('/maintenance')
          break
        case 502:
        case 504:
          messages.next('Server not reachable.')
        // opts.auth.logout()
      }
    }
    Promise.reject(error)
  })

  return axios
}
