const CACHE = 'home-control-v2'

self.addEventListener('install', e => {
  e.waitUntil(
    caches.open(CACHE).then(c => c.addAll(['./', './index.html']))
  )
  self.skipWaiting()
})

self.addEventListener('activate', e => {
  e.waitUntil(
    caches.keys().then(keys =>
      Promise.all(keys.filter(k => k !== CACHE).map(k => caches.delete(k)))
    )
  )
  self.clients.claim()
})

self.addEventListener('fetch', e => {
  if (!e.request.url.startsWith(self.location.origin)) return
  if (e.request.method !== 'GET') return

  // Network-first: always fetch from server; fall back to cache only when offline.
  e.respondWith(
    fetch(e.request)
      .then(resp => {
        if (resp.ok) {
          caches.open(CACHE).then(c => c.put(e.request, resp.clone()))
        }
        return resp
      })
      .catch(() => caches.match(e.request))
  )
})
