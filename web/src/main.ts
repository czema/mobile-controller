import './assets/main.css'
import { library } from '@fortawesome/fontawesome-svg-core'
import { fas } from '@fortawesome/free-solid-svg-icons'
import { far } from '@fortawesome/free-regular-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'
import { createApp } from 'vue'
import App from './App.vue'

library.add(fas, far)

const app = createApp(App)
app.component('font-awesome-icon', FontAwesomeIcon)
app.mount('#app')

if ('serviceWorker' in navigator) {
  navigator.serviceWorker.register('./sw.js').catch(() => {})
}

// iOS Safari scrolls the document when the soft keyboard appears, leaving a
// non-zero scrollY after the keyboard closes.  Touch events are then reported
// in document coordinates while fixed elements are in viewport coordinates,
// causing every tap to be offset by the residual scroll amount.
// Belt-and-suspenders reset alongside the body { position: fixed } in base.css.
document.addEventListener('focusout', () => {
  window.requestAnimationFrame(() => {
    if (window.scrollX !== 0 || window.scrollY !== 0) {
      window.scrollTo(0, 0)
    }
  })
})
