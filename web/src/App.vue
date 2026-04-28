<script setup lang="ts">
import { ref, computed, watch, onUnmounted } from 'vue'
import type { Config, Room, Device } from './types'
import DeviceButton from './components/DeviceButton.vue'

// ─── Constants ───────────────────────────────────────────────────────────────

const DEVICE_ID = '33003d000947373034353237'
const PARTICLE_ACCESS_TOKEN = '07f50bb760a40364335eaa07543a2a933f76fece'

const COLS = 8
const ROWS = 6
const MAX_COL = COLS - 2   // reserve last column for FABs
const MAX_ROW = ROWS - 2   // reserve last row for FABs

const ICON_OPTIONS = [
  { name: 'Power',        icon: 'fa-solid fa-power-off' },
  { name: 'Lightbulb',    icon: 'fa-solid fa-lightbulb' },
  { name: 'TV',           icon: 'fa-solid fa-tv' },
  { name: 'Fan',          icon: 'fa-solid fa-fan' },
  { name: 'Vol Up',       icon: 'fa-solid fa-volume-high' },
  { name: 'Vol Down',     icon: 'fa-solid fa-volume-low' },
  { name: 'Mute',         icon: 'fa-solid fa-volume-xmark' },
  { name: 'Play',         icon: 'fa-solid fa-play' },
  { name: 'Pause',        icon: 'fa-solid fa-pause' },
  { name: 'Stop',         icon: 'fa-solid fa-stop' },
  { name: 'Skip Fwd',     icon: 'fa-solid fa-forward-step' },
  { name: 'Skip Back',    icon: 'fa-solid fa-backward-step' },
  { name: 'Fast Fwd',     icon: 'fa-solid fa-forward' },
  { name: 'Rewind',       icon: 'fa-solid fa-backward' },
  { name: 'Music',        icon: 'fa-solid fa-music' },
  { name: 'Film',         icon: 'fa-solid fa-film' },
  { name: 'Radio',        icon: 'fa-solid fa-radio' },
  { name: 'Up',           icon: 'fa-solid fa-chevron-up' },
  { name: 'Down',         icon: 'fa-solid fa-chevron-down' },
  { name: 'Left',         icon: 'fa-solid fa-chevron-left' },
  { name: 'Right',        icon: 'fa-solid fa-chevron-right' },
  { name: 'OK',           icon: 'fa-solid fa-circle-check' },
  { name: 'Menu',         icon: 'fa-solid fa-bars' },
  { name: 'Back',         icon: 'fa-solid fa-arrow-left' },
  { name: 'Home',         icon: 'fa-solid fa-house' },
  { name: 'Info',         icon: 'fa-solid fa-circle-info' },
  { name: 'Settings',     icon: 'fa-solid fa-gear' },
  { name: 'Thermometer',  icon: 'fa-solid fa-temperature-half' },
  { name: 'Snowflake',    icon: 'fa-solid fa-snowflake' },
  { name: 'Fire',         icon: 'fa-solid fa-fire' },
  { name: 'Wind',         icon: 'fa-solid fa-wind' },
  { name: 'Sun',          icon: 'fa-solid fa-sun' },
  { name: 'Moon',         icon: 'fa-solid fa-moon' },
  { name: 'Bed',          icon: 'fa-solid fa-bed' },
  { name: 'Couch',        icon: 'fa-solid fa-couch' },
  { name: 'Door',         icon: 'fa-solid fa-door-open' },
  { name: 'Lock',         icon: 'fa-solid fa-lock' },
  { name: 'Plug',         icon: 'fa-solid fa-plug' },
  { name: 'Bell',         icon: 'fa-solid fa-bell' },
  { name: 'Bolt',         icon: 'fa-solid fa-bolt' },
  { name: 'Camera',       icon: 'fa-solid fa-camera' },
  { name: 'Gamepad',      icon: 'fa-solid fa-gamepad' },
  { name: 'Ch +',         icon: 'fa-solid fa-circle-plus' },
  { name: 'Ch -',         icon: 'fa-solid fa-circle-minus' },
  { name: 'Record',       icon: 'fa-solid fa-circle' },
  { name: 'Star',         icon: 'fa-solid fa-star' },
  { name: 'WiFi',         icon: 'fa-solid fa-wifi' },
  { name: 'Speaker',      icon: 'fa-solid fa-tower-broadcast' },
]

const ROOM_COLORS = [
  '#60bce0', '#70e090', '#e06868', '#f4a860',
  '#a060e0', '#e060b0', '#60e0b0', '#e0d060',
  '#8090c0', '#c0a060', '#60a0e0', '#e08060',
]

// ─── Config / storage ────────────────────────────────────────────────────────

function loadConfig(): Config {
  try {
    const raw = JSON.parse(window.localStorage.getItem('config') ?? '{}')
    if (raw?.version === 2 && Array.isArray(raw.rooms) && Array.isArray(raw.devices)) {
      const cfg = raw as Config
      if (cfg.rooms.length === 0) {
        cfg.rooms.push({ id: crypto.randomUUID(), name: 'Bedroom', color: '#60bce0' })
      }
      return cfg
    }
  } catch {}
  return {
    version: 2,
    rooms: [{ id: crypto.randomUUID(), name: 'Bedroom', color: '#60bce0' }],
    devices: [],
  }
}

const config = ref<Config>(loadConfig())

const saveConfig = () => {
  window.localStorage.setItem('config', JSON.stringify(config.value))
}

// ─── Navigation ──────────────────────────────────────────────────────────────

const sidebarSlim = ref(false)
const currentRoomId = ref<string | null>(config.value.rooms[0]?.id ?? null)

const currentRoom = computed(() =>
  config.value.rooms.find(r => r.id === currentRoomId.value) ?? null
)

const currentRoomDevices = computed(() =>
  config.value.devices.filter(d => d.roomId === currentRoomId.value)
)

// ─── Canvas sizing + grid ─────────────────────────────────────────────────────

const canvasRef = ref<HTMLElement | null>(null)
const canvasW = ref(0)
const canvasH = ref(0)

const gridCellW = computed(() => canvasW.value > 0 ? canvasW.value / COLS : 80)
const gridCellH = computed(() => canvasH.value > 0 ? canvasH.value / ROWS : 96)
const deviceIconSize = computed(() => Math.min(Math.floor(gridCellW.value * 0.72), 64))

const hexToRgb = (hex: string) => ({
  r: parseInt(hex.slice(1, 3), 16),
  g: parseInt(hex.slice(3, 5), 16),
  b: parseInt(hex.slice(5, 7), 16),
})

const canvasStyle = computed(() => {
  const color = currentRoom.value?.color ?? '#60bce0'
  const { r, g, b } = hexToRgb(color)
  const mix = (ch: number, base: number) => Math.round(base + (ch - base) * 0.12)
  return { backgroundColor: `rgb(${mix(r, 24)}, ${mix(g, 24)}, ${mix(b, 36)})` }
})

const roomItemStyle = (room: Room): Record<string, string> => {
  if (currentRoomId.value !== room.id || sidebarSlim.value || !room.color) return {}
  const { r, g, b } = hexToRgb(room.color)
  return {
    background: `rgba(${r},${g},${b},0.18)`,
    color: room.color,
  }
}

const snapToGrid = (px: number, py: number): { x: number; y: number } => {
  const cw = gridCellW.value
  const ch = gridCellH.value
  const col = Math.min(Math.max(0, Math.round(px / cw)), MAX_COL)
  const row = Math.min(Math.max(0, Math.round(py / ch)), MAX_ROW)
  return { x: col * cw, y: row * ch }
}

const snapAllDevices = () => {
  if (!canvasW.value || !canvasH.value) return
  let changed = false
  config.value.devices.forEach(d => {
    const { x, y } = snapToGrid(d.x, d.y)
    if (x !== d.x || y !== d.y) {
      d.x = x
      d.y = y
      changed = true
    }
  })
  if (changed) saveConfig()
}

const ro = new ResizeObserver(entries => {
  for (const e of entries) {
    canvasW.value = e.contentRect.width
    canvasH.value = e.contentRect.height
    snapAllDevices()
  }
})

watch(canvasRef, el => {
  ro.disconnect()
  if (el) ro.observe(el)
}, { immediate: true })

onUnmounted(() => ro.disconnect())

// ─── Slot management ─────────────────────────────────────────────────────────

const getLowestAvailableSlot = (type: 'ir' | 'rf', excludeId?: string): number => {
  const used = new Set(
    config.value.devices
      .filter(d => d.type === type && d.id !== excludeId)
      .map(d => d.slot)
  )
  for (let i = 0; i < 64; i++) {
    if (!used.has(i)) return i
  }
  return -1
}

const getDefaultPosition = (roomId: string): { x: number; y: number } => {
  const cw = gridCellW.value
  const ch = gridCellH.value
  const existing = config.value.devices.filter(d => d.roomId === roomId)

  for (let row = 0; row <= MAX_ROW; row++) {
    for (let col = 0; col <= MAX_COL; col++) {
      const x = col * cw
      const y = row * ch
      const occupied = existing.some(d =>
        Math.abs(d.x - x) < cw / 2 && Math.abs(d.y - y) < ch / 2
      )
      if (!occupied) return { x, y }
    }
  }
  return { x: 0, y: 0 }
}

// ─── Particle API ────────────────────────────────────────────────────────────

const callParticle = async (func: string, arg: string | number): Promise<number> => {
  try {
    const resp = await fetch(`https://api.particle.io/v1/devices/${DEVICE_ID}/${func}`, {
      method: 'POST',
      body: new URLSearchParams({ arg: String(arg) }),
      headers: { Authorization: `Bearer ${PARTICLE_ACCESS_TOKEN}` },
    })
    const data = await resp.json()
    return data.return_value ?? -99
  } catch {
    return -99
  }
}

// ─── Toast ───────────────────────────────────────────────────────────────────

const toast = ref<{ msg: string; type: 'ok' | 'err' | 'info' } | null>(null)
let toastTimer = 0

const showToast = (msg: string, type: 'ok' | 'err' | 'info' = 'info') => {
  toast.value = { msg, type }
  window.clearTimeout(toastTimer)
  toastTimer = window.setTimeout(() => { toast.value = null }, 3200)
}

// ─── Device actions ──────────────────────────────────────────────────────────

const onDeviceTap = async (device: Device) => {
  const ret = await callParticle(`${device.type}_send`, device.slot)
  if (ret === 1) {
    const d = config.value.devices.find(d => d.id === device.id)
    if (d) { d.lastSent = new Date().toISOString(); saveConfig() }
  } else if (ret === -2) {
    showToast('Slot not programmed — long-press to configure', 'err')
  } else if (ret === -99) {
    showToast('Could not reach the Photon', 'err')
  } else {
    showToast(`Send failed (${ret})`, 'err')
  }
}

const onDeviceMoved = (id: string, x: number, y: number) => {
  const d = config.value.devices.find(d => d.id === id)
  if (d) { d.x = x; d.y = y; saveConfig() }
}

// ─── Device dialog ───────────────────────────────────────────────────────────

type DeviceDialogMode = 'add' | 'edit' | null

const deviceDialogRef = ref<HTMLDialogElement | null>(null)
const deviceDialogMode = ref<DeviceDialogMode>(null)
const editingDevice = ref<Partial<Device> & { roomId: string } | null>(null)
const programState = ref<'idle' | 'ready' | 'err'>('idle')
let programStateTimer = 0

const isNewDevice = computed(() => deviceDialogMode.value === 'add')
const canProgramDevice = computed(() => !!editingDevice.value?.type)
const canSaveDevice = computed(() =>
  !!(editingDevice.value?.name?.trim()) && !!editingDevice.value?.type
)
const saveButtonLabel = computed(() => {
  const d = editingDevice.value
  if (!d?.type || d.slot === undefined) return 'Save'
  return `Save to ${d.type.toUpperCase()} Slot ${d.slot}`
})

const openAddDevice = (type: 'ir' | 'rf') => {
  if (!currentRoomId.value) return
  const slot = getLowestAvailableSlot(type)
  editingDevice.value = {
    roomId: currentRoomId.value,
    name: '',
    type,
    slot,
    x: 0,
    y: 0,
    iconName: 'fa-solid fa-circle',
    lastSent: null,
  }
  deviceDialogMode.value = 'add'
  programState.value = 'idle'
  deviceDialogRef.value?.showModal()
}

const openEditDevice = (device: Device) => {
  editingDevice.value = { ...device }
  deviceDialogMode.value = 'edit'
  programState.value = 'idle'
  deviceDialogRef.value?.showModal()
}

const programDevice = async () => {
  const d = editingDevice.value
  if (!d?.type || d.slot === undefined) return
  const ret = await callParticle(`${d.type}_recv`, d.slot)
  window.clearTimeout(programStateTimer)
  if (ret === 1) {
    programState.value = 'ready'
    programStateTimer = window.setTimeout(() => { programState.value = 'idle' }, 8000)
  } else {
    programState.value = 'err'
    programStateTimer = window.setTimeout(() => { programState.value = 'idle' }, 4000)
  }
}

const saveDevice = () => {
  const d = editingDevice.value
  if (!d || !d.name?.trim() || !d.type) return

  if (isNewDevice.value) {
    const pos = getDefaultPosition(d.roomId)
    config.value.devices.push({
      id: crypto.randomUUID(),
      roomId: d.roomId,
      name: d.name.trim(),
      type: d.type,
      slot: d.slot ?? getLowestAvailableSlot(d.type),
      x: pos.x,
      y: pos.y,
      iconName: d.iconName || 'fa-solid fa-circle',
      lastSent: null,
    })
  } else {
    const existing = config.value.devices.find(dev => dev.id === d.id)
    if (existing) {
      existing.name = d.name.trim()
      existing.iconName = d.iconName || existing.iconName
    }
  }

  saveConfig()
  closeDeviceDialog()
}

const deleteDevice = () => {
  const d = editingDevice.value
  if (!d?.id) return
  const name = config.value.devices.find(dev => dev.id === d.id)?.name ?? 'this device'
  if (!window.confirm(`Delete "${name}"?`)) return
  config.value.devices = config.value.devices.filter(dev => dev.id !== d.id)
  saveConfig()
  closeDeviceDialog()
}

const closeDeviceDialog = () => {
  editingDevice.value = null
  deviceDialogMode.value = null
  programState.value = 'idle'
  window.clearTimeout(programStateTimer)
  deviceDialogRef.value?.close()
}

// ─── Room dialog ─────────────────────────────────────────────────────────────

type RoomDialogMode = 'add' | 'edit' | null

const roomDialogRef = ref<HTMLDialogElement | null>(null)
const roomDialogMode = ref<RoomDialogMode>(null)
const editingRoom = ref<Partial<Room> | null>(null)

const isNewRoom = computed(() => roomDialogMode.value === 'add')
const canSaveRoom = computed(() => !!(editingRoom.value?.name?.trim()))

const openAddRoom = () => {
  editingRoom.value = { name: '', color: '#60bce0' }
  roomDialogMode.value = 'add'
  roomDialogRef.value?.showModal()
}

const openEditRoom = (room: Room) => {
  editingRoom.value = { ...room, color: room.color || '#60bce0' }
  roomDialogMode.value = 'edit'
  roomDialogRef.value?.showModal()
}

const saveRoom = () => {
  const r = editingRoom.value
  if (!r?.name?.trim()) return

  if (isNewRoom.value) {
    const newRoom: Room = {
      id: crypto.randomUUID(),
      name: r.name.trim(),
      color: r.color || '#60bce0',
    }
    config.value.rooms.push(newRoom)
    currentRoomId.value = newRoom.id
  } else if (r.id) {
    const existing = config.value.rooms.find(room => room.id === r.id)
    if (existing) {
      existing.name = r.name.trim()
      existing.color = r.color || existing.color || '#60bce0'
    }
  }

  saveConfig()
  closeRoomDialog()
}

const deleteRoom = () => {
  const r = editingRoom.value
  if (!r?.id) return
  const devCount = config.value.devices.filter(d => d.roomId === r.id).length
  const msg = devCount > 0
    ? `Delete "${r.name}"? This will also remove ${devCount} device${devCount > 1 ? 's' : ''}.`
    : `Delete room "${r.name}"?`
  if (!window.confirm(msg)) return

  config.value.devices = config.value.devices.filter(d => d.roomId !== r.id)
  config.value.rooms = config.value.rooms.filter(room => room.id !== r.id)

  if (currentRoomId.value === r.id) {
    currentRoomId.value = config.value.rooms[0]?.id ?? null
  }

  saveConfig()
  closeRoomDialog()
}

const closeRoomDialog = () => {
  editingRoom.value = null
  roomDialogMode.value = null
  roomDialogRef.value?.close()
}
</script>

<template>
  <!-- ── Toast ─────────────────────────────────────────────────────────────── -->
  <transition name="toast">
    <div v-if="toast" class="toast-msg" :class="`toast-msg--${toast.type}`">
      {{ toast.msg }}
    </div>
  </transition>

  <!-- ── Device dialog ──────────────────────────────────────────────────────── -->
  <dialog ref="deviceDialogRef" class="hc-dialog" @click.self="closeDeviceDialog" @cancel.prevent="closeDeviceDialog">
    <div class="hc-dialog__header">
      <h3>{{ isNewDevice ? 'Add Device' : 'Edit Device' }}</h3>
      <button class="hc-dialog__close" @click="closeDeviceDialog">
        <font-awesome-icon icon="fa-solid fa-xmark" />
      </button>
    </div>

    <div class="hc-dialog__body" v-if="editingDevice">
      <!-- Name -->
      <div class="form-row">
        <label class="form-label">Name</label>
        <input v-model="editingDevice.name" type="text" class="form-input"
               placeholder="e.g. TV Power" autocomplete="off" />
      </div>

      <!-- Icon picker -->
      <div class="form-row">
        <label class="form-label">Icon</label>
        <div class="icon-grid">
          <button v-for="opt in ICON_OPTIONS" :key="opt.icon"
                  class="icon-opt"
                  :class="{ 'icon-opt--selected': editingDevice.iconName === opt.icon }"
                  :title="opt.name"
                  @click="editingDevice.iconName = opt.icon">
            <font-awesome-icon :icon="opt.icon" />
          </button>
        </div>
      </div>

      <!-- Program status -->
      <div v-if="programState !== 'idle'" class="program-status"
           :class="`program-status--${programState}`">
        <font-awesome-icon :icon="programState === 'ready' ? 'fa-solid fa-circle-check' : 'fa-solid fa-triangle-exclamation'" />
        {{ programState === 'ready'
          ? 'Press your remote button now, then tap Save.'
          : 'Could not reach the Photon' }}
      </div>
    </div>

    <div class="hc-dialog__footer">
      <button class="btn-secondary" :disabled="!canProgramDevice" @click="programDevice">
        <font-awesome-icon icon="fa-solid fa-satellite-dish" /> Program
      </button>
      <div class="flex-gap"></div>
      <button v-if="!isNewDevice" class="btn-danger" @click="deleteDevice">Delete</button>
      <button class="btn-primary" :disabled="!canSaveDevice" @click="saveDevice">{{ saveButtonLabel }}</button>
    </div>
  </dialog>

  <!-- ── Room dialog ────────────────────────────────────────────────────────── -->
  <dialog ref="roomDialogRef" class="hc-dialog hc-dialog--room" @click.self="closeRoomDialog" @cancel.prevent="closeRoomDialog">
    <div class="hc-dialog__header">
      <h3>{{ isNewRoom ? 'Add Room' : 'Edit Room' }}</h3>
      <button class="hc-dialog__close" @click="closeRoomDialog">
        <font-awesome-icon icon="fa-solid fa-xmark" />
      </button>
    </div>

    <div class="hc-dialog__body" v-if="editingRoom">
      <div class="form-row">
        <label class="form-label">Room Name</label>
        <input v-model="editingRoom.name" type="text" class="form-input"
               placeholder="e.g. Living Room" autocomplete="off"
               @keyup.enter="saveRoom" />
      </div>
      <div class="form-row">
        <label class="form-label">Color</label>
        <div class="color-swatches">
          <button
            v-for="c in ROOM_COLORS" :key="c"
            class="color-swatch"
            :class="{ 'color-swatch--selected': editingRoom.color === c }"
            :style="{ background: c }"
            @click="editingRoom.color = c"
          />
        </div>
      </div>
    </div>

    <div class="hc-dialog__footer">
      <button v-if="!isNewRoom" class="btn-danger" @click="deleteRoom">Delete Room</button>
      <div class="flex-gap"></div>
      <button class="btn-secondary" @click="closeRoomDialog">Cancel</button>
      <button class="btn-primary" :disabled="!canSaveRoom" @click="saveRoom">Save</button>
    </div>
  </dialog>

  <!-- ── App shell ──────────────────────────────────────────────────────────── -->
  <div class="app">

    <!-- Sidebar -->
    <nav class="sidebar" :class="{ 'sidebar--slim': sidebarSlim }">

      <!-- Brand / toggle — the entire area is the click target -->
      <button class="sidebar__top" @click="sidebarSlim = !sidebarSlim"
              :title="sidebarSlim ? 'Expand' : 'Collapse'">
        <font-awesome-icon icon="fa-solid fa-house" class="sidebar__logo" />
        <span v-if="!sidebarSlim" class="sidebar__title">Home Control</span>
      </button>

      <div class="sidebar__divider"></div>

      <div class="sidebar__rooms">
        <button
          v-for="room in config.rooms"
          :key="room.id"
          class="room-item"
          :class="{ 'room-item--active': currentRoomId === room.id }"
          :style="roomItemStyle(room)"
          @click="currentRoomId = room.id"
          :title="sidebarSlim ? room.name : undefined"
        >
          <span
            class="room-item__dot"
            :style="currentRoomId === room.id && room.color
              ? { background: room.color, boxShadow: `0 0 6px ${room.color}b0` }
              : {}"
          ></span>
          <span v-if="!sidebarSlim" class="room-item__name">{{ room.name }}</span>
          <button v-if="!sidebarSlim" class="room-item__menu" @click.stop="openEditRoom(room)" title="Edit room">
            <font-awesome-icon icon="fa-solid fa-ellipsis-vertical" />
          </button>
        </button>
      </div>

      <div class="sidebar__divider"></div>

      <button class="sidebar__add-room" @click="openAddRoom" :title="sidebarSlim ? 'Add Room' : undefined">
        <font-awesome-icon icon="fa-solid fa-plus" class="sidebar__add-icon" />
        <span v-if="!sidebarSlim">Add Room</span>
      </button>
    </nav>

    <!-- Main -->
    <main class="main">
      <template v-if="currentRoom">
        <header class="room-header">
          <span class="room-header__name">{{ currentRoom.name }}</span>
          <button class="room-header__edit" @click="openEditRoom(currentRoom)" title="Edit room">
            <font-awesome-icon icon="fa-solid fa-gear" />
          </button>
        </header>

        <div class="canvas-wrap">
          <div class="canvas" ref="canvasRef" :style="canvasStyle">
            <DeviceButton
              v-for="device in currentRoomDevices"
              :key="device.id"
              :device="device"
              :canvas-width="canvasW"
              :canvas-height="canvasH"
              :grid-cell-w="gridCellW"
              :grid-cell-h="gridCellH"
              :icon-size="deviceIconSize"
              @tap="onDeviceTap"
              @longpress="openEditDevice"
              @moved="onDeviceMoved"
            />
            <div v-if="currentRoomDevices.length === 0" class="canvas__hint">
              <font-awesome-icon icon="fa-solid fa-hand-pointer" class="canvas__hint-icon" />
              <p>Tap <strong>+</strong> to add your first device</p>
            </div>
          </div>

          <div class="fab-group">
            <button class="fab-pill fab-pill--ir" @click="openAddDevice('ir')" aria-label="Add IR device">
              <font-awesome-icon icon="fa-solid fa-tower-broadcast" />
              <span>IR</span>
            </button>
            <button class="fab-pill fab-pill--rf" @click="openAddDevice('rf')" aria-label="Add RF device">
              <font-awesome-icon icon="fa-solid fa-wifi" />
              <span>RF</span>
            </button>
          </div>
        </div>
      </template>
    </main>
  </div>
</template>

<style scoped>
/* ── App shell ────────────────────────────────────────────────────────────── */
.app {
  display: flex;
  /* inset: 0 + position: fixed anchors the layout to the viewport so iOS
     keyboard appearance/disappearance never shifts touch-event coordinates */
  position: fixed;
  inset: 0;
  overflow: hidden;
  background: #0b0b13;
  color: #e8e8f4;
  font-family: Inter, -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
}

/* ── Sidebar ──────────────────────────────────────────────────────────────── */
.sidebar {
  display: flex;
  flex-direction: column;
  width: 210px;
  min-width: 210px;
  background: #11111c;
  border-right: 1px solid rgba(255, 255, 255, 0.05);
  transition: width 0.28s ease, min-width 0.28s ease;
  overflow: hidden;
  flex-shrink: 0;
}

.sidebar--slim {
  width: 52px;
  min-width: 52px;
}

/* Brand row is the collapse toggle */
.sidebar__top {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 18px 14px 14px;
  background: none;
  border: none;
  cursor: pointer;
  width: 100%;
  overflow: hidden;
  flex-shrink: 0;
  transition: background 0.15s;
  text-align: left;
}

.sidebar--slim .sidebar__top {
  justify-content: center;
  padding: 18px 0 14px;
}

.sidebar__top:hover {
  background: rgba(255, 255, 255, 0.05);
}

.sidebar__logo {
  font-size: 1.15rem;
  color: #60bce0;
  flex-shrink: 0;
}

.sidebar__title {
  font-size: 0.88rem;
  font-weight: 700;
  color: #d8d8ec;
  letter-spacing: 0.6px;
  white-space: nowrap;
  overflow: hidden;
}

.sidebar__divider {
  height: 1px;
  margin: 0 12px;
  background: rgba(255, 255, 255, 0.05);
  flex-shrink: 0;
}

.sidebar__rooms {
  flex: 1;
  overflow-y: auto;
  padding: 10px 0;
}

.room-item {
  display: flex;
  align-items: center;
  gap: 10px;
  width: calc(100% - 12px);
  padding: 9px 12px;
  border: none;
  border-radius: 8px;
  margin: 2px 6px;
  background: none;
  color: #9898b8;
  font-size: 0.9rem;
  font-weight: 500;
  cursor: pointer;
  text-align: left;
  transition: background 0.15s, color 0.15s;
  white-space: nowrap;
  overflow: hidden;
}

.room-item:hover {
  background: rgba(255, 255, 255, 0.05);
  color: #c0c0d8;
}

.room-item--active {
  background: rgba(96, 188, 224, 0.12);
  color: #b8e0f8;
}

/* In slim mode, no background tint on active — just the glowing dot */
.sidebar--slim .room-item--active {
  background: none;
}

.room-item__dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: #404050;
  flex-shrink: 0;
  transition: background 0.15s, box-shadow 0.15s;
}

.room-item__name {
  flex: 1;
  overflow: hidden;
  text-overflow: ellipsis;
}

.room-item__menu {
  background: none;
  border: none;
  color: #505060;
  cursor: pointer;
  padding: 2px 5px;
  border-radius: 4px;
  font-size: 0.85rem;
  opacity: 0;
  transition: opacity 0.15s, color 0.15s;
  flex-shrink: 0;
  line-height: 1;
}

.room-item:hover .room-item__menu {
  opacity: 1;
}

.room-item__menu:hover {
  color: #a0a0a0;
}

.sidebar__add-room {
  display: flex;
  align-items: center;
  gap: 10px;
  width: 100%;
  padding: 10px 14px;
  border: none;
  background: none;
  color: #9090b8;
  font-size: 0.9rem;
  cursor: pointer;
  text-align: left;
  transition: color 0.15s, background 0.15s;
  white-space: nowrap;
  overflow: hidden;
  flex-shrink: 0;
}

.sidebar--slim .sidebar__add-room {
  justify-content: center;
  padding: 10px 0;
}

.sidebar__add-room:hover {
  color: #b0b0d0;
  background: rgba(255, 255, 255, 0.04);
}

.sidebar__add-icon {
  flex-shrink: 0;
  font-size: 0.85rem;
}

/* ── Main ─────────────────────────────────────────────────────────────────── */
.main {
  flex: 1;
  display: flex;
  flex-direction: column;
  padding: 16px 20px 20px;
  gap: 10px;
  overflow: hidden;
  min-width: 0;
}

/* ── Room header ──────────────────────────────────────────────────────────── */
.room-header {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;
  flex-shrink: 0;
}

.room-header__name {
  font-size: 0.8rem;
  font-weight: 700;
  letter-spacing: 2.5px;
  text-transform: uppercase;
  color: #b0b0cc;
}

.room-header__edit {
  background: none;
  border: none;
  color: #686880;
  font-size: 0.9rem;
  cursor: pointer;
  padding: 3px 6px;
  border-radius: 6px;
  transition: color 0.15s;
}

.room-header__edit:hover {
  color: #a0a0b8;
}

/* ── Canvas wrapper + canvas ─────────────────────────────────────────────── */
.canvas-wrap {
  flex: 1;
  position: relative;
  min-height: 0;
}

.canvas {
  width: 100%;
  height: 100%;
  position: relative;
  border-radius: 22px;
  border: 1px solid rgba(255, 255, 255, 0.05);
  background-image:
    radial-gradient(circle, rgba(255, 255, 255, 0.03) 1px, transparent 1px);
  background-size: 26px 26px;
  box-shadow:
    inset 0 2px 10px rgba(0, 0, 0, 0.75),
    inset 0 -1px 2px rgba(255, 255, 255, 0.02);
  overflow: hidden;
  transition: background-color 0.4s ease;
}

.canvas__hint {
  position: absolute;
  inset: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 12px;
  pointer-events: none;
  color: #8888a8;
  font-size: 0.95rem;
  text-align: center;
}

.canvas__hint-icon {
  font-size: 2.2rem;
  color: #686880;
}

/* ── FAB group ────────────────────────────────────────────────────────────── */
.fab-group {
  position: absolute;
  bottom: 22px;
  right: 22px;
  display: flex;
  flex-direction: column;
  gap: 8px;
  align-items: flex-end;
  z-index: 10;
}

.fab-pill {
  display: flex;
  align-items: center;
  gap: 7px;
  padding: 10px 18px;
  border-radius: 24px;
  border: none;
  color: #fff;
  font-size: 0.9rem;
  font-weight: 700;
  cursor: pointer;
  font-family: inherit;
  transition: transform 0.15s, box-shadow 0.15s, opacity 0.15s;
}

.fab-pill:hover {
  transform: scale(1.07);
  opacity: 0.92;
}

.fab-pill:active {
  transform: scale(0.93);
}

.fab-pill--ir {
  background: linear-gradient(135deg, #5abce0, #3090c0);
  box-shadow: 0 4px 18px rgba(0, 160, 210, 0.4);
}

.fab-pill--rf {
  background: linear-gradient(135deg, #f0a050, #c07030);
  box-shadow: 0 4px 18px rgba(200, 120, 40, 0.4);
}

/* ── Dialog ───────────────────────────────────────────────────────────────── */
.hc-dialog {
  background: #1a1a26;
  border: 1px solid rgba(255, 255, 255, 0.09);
  border-radius: 20px;
  color: #e8e8f4;
  padding: 0;
  max-width: 500px;
  width: 92vw;
  box-shadow: 0 24px 64px rgba(0, 0, 0, 0.85);
  margin: auto;
  outline: none;
}

.hc-dialog--room {
  max-width: 400px;
}

.hc-dialog::backdrop {
  background: rgba(0, 0, 0, 0.72);
  backdrop-filter: blur(5px);
}

.hc-dialog__header {
  display: flex;
  align-items: center;
  padding: 20px 22px 0;
}

.hc-dialog__header h3 {
  flex: 1;
  margin: 0;
  font-size: 1.05rem;
  font-weight: 700;
  color: #dcdcf0;
}

.hc-dialog__close {
  background: none;
  border: none;
  color: #686880;
  font-size: 1.05rem;
  cursor: pointer;
  padding: 4px 8px;
  border-radius: 6px;
  line-height: 1;
  transition: color 0.15s;
}

.hc-dialog__close:hover {
  color: #a8a8c0;
}

.hc-dialog__body {
  padding: 18px 22px 4px;
  display: flex;
  flex-direction: column;
  gap: 18px;
  max-height: 65vh;
  overflow-y: auto;
}

.hc-dialog__footer {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 16px 22px 20px;
  border-top: 1px solid rgba(255, 255, 255, 0.06);
  margin-top: 14px;
}

.flex-gap { flex: 1; }

/* ── Form elements ────────────────────────────────────────────────────────── */
.form-row {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.form-label {
  font-size: 0.75rem;
  font-weight: 700;
  color: #6868a0;
  text-transform: uppercase;
  letter-spacing: 1px;
}

.form-input {
  background: #10101a;
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 10px;
  color: #dcdcf0;
  padding: 10px 14px;
  font-size: 0.95rem;
  outline: none;
  width: 100%;
  transition: border-color 0.15s;
  font-family: inherit;
}

.form-input:focus {
  border-color: rgba(96, 188, 224, 0.5);
}

.form-input::placeholder {
  color: #404060;
}

/* Icon grid — tall enough to show 2.5 rows as a scroll hint */
.icon-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(42px, 1fr));
  gap: 6px;
  height: 117px;
  overflow-y: auto;
}

.icon-opt {
  aspect-ratio: 1;
  border-radius: 9px;
  border: 1px solid rgba(255, 255, 255, 0.07);
  background: #10101a;
  color: #6868a0;
  font-size: 1.05rem;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: background 0.12s, border-color 0.12s, color 0.12s;
}

.icon-opt:hover {
  background: rgba(255, 255, 255, 0.08);
  color: #b0b0c8;
}

.icon-opt--selected {
  background: rgba(96, 188, 224, 0.18);
  border-color: rgba(96, 188, 224, 0.5);
  color: #7de0f4;
}

/* Color swatches */
.color-swatches {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}

.color-swatch {
  width: 22px;
  height: 22px;
  border-radius: 50%;
  border: 2px solid transparent;
  cursor: pointer;
  padding: 0;
  transition: transform 0.1s, border-color 0.12s;
  outline: none;
}

.color-swatch:hover {
  transform: scale(1.2);
}

.color-swatch--selected {
  border-color: #fff;
  transform: scale(1.2);
}

/* Program status */
.program-status {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 10px 14px;
  border-radius: 10px;
  font-size: 0.9rem;
  font-weight: 500;
}

.program-status--ready {
  background: rgba(50, 200, 100, 0.12);
  color: #80f0a0;
  border: 1px solid rgba(50, 200, 100, 0.25);
}

.program-status--err {
  background: rgba(220, 60, 60, 0.1);
  color: #f08080;
  border: 1px solid rgba(220, 60, 60, 0.25);
}

/* ── Buttons ──────────────────────────────────────────────────────────────── */
.btn-primary {
  display: inline-flex;
  align-items: center;
  gap: 7px;
  padding: 9px 20px;
  border-radius: 10px;
  border: none;
  background: linear-gradient(135deg, #5abce0, #3090c0);
  color: #fff;
  font-size: 0.9rem;
  font-weight: 700;
  cursor: pointer;
  font-family: inherit;
  transition: opacity 0.15s, transform 0.1s;
  white-space: nowrap;
}

.btn-primary:disabled {
  opacity: 0.35;
  cursor: not-allowed;
}

.btn-primary:not(:disabled):hover {
  opacity: 0.9;
}

.btn-primary:not(:disabled):active {
  transform: scale(0.96);
}

.btn-secondary {
  display: inline-flex;
  align-items: center;
  gap: 7px;
  padding: 9px 16px;
  border-radius: 10px;
  border: 1px solid rgba(255, 255, 255, 0.12);
  background: rgba(255, 255, 255, 0.06);
  color: #a8a8c0;
  font-size: 0.9rem;
  font-weight: 600;
  cursor: pointer;
  font-family: inherit;
  transition: background 0.15s, color 0.15s;
  white-space: nowrap;
}

.btn-secondary:disabled {
  opacity: 0.35;
  cursor: not-allowed;
}

.btn-secondary:not(:disabled):hover {
  background: rgba(255, 255, 255, 0.1);
  color: #d0d0e8;
}

.btn-danger {
  display: inline-flex;
  align-items: center;
  gap: 7px;
  padding: 9px 16px;
  border-radius: 10px;
  border: 1px solid rgba(220, 60, 60, 0.3);
  background: rgba(220, 60, 60, 0.08);
  color: #f08080;
  font-size: 0.9rem;
  font-weight: 600;
  cursor: pointer;
  font-family: inherit;
  transition: background 0.15s;
  white-space: nowrap;
}

.btn-danger:hover {
  background: rgba(220, 60, 60, 0.18);
}

/* ── Toast ────────────────────────────────────────────────────────────────── */
.toast-msg {
  position: fixed;
  bottom: 28px;
  left: 50%;
  transform: translateX(-50%);
  padding: 11px 22px;
  border-radius: 12px;
  font-size: 0.9rem;
  font-weight: 600;
  z-index: 9999;
  pointer-events: none;
  white-space: nowrap;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.5);
}

.toast-msg--ok   { background: rgba(40, 180, 90, 0.92);  color: #fff; }
.toast-msg--err  { background: rgba(200, 50, 50, 0.92);  color: #fff; }
.toast-msg--info { background: rgba(50, 140, 200, 0.92); color: #fff; }

.toast-enter-active, .toast-leave-active {
  transition: opacity 0.28s ease, transform 0.28s ease;
}

.toast-enter-from {
  opacity: 0;
  transform: translateX(-50%) translateY(14px);
}

.toast-leave-to {
  opacity: 0;
  transform: translateX(-50%) translateY(-10px);
}
</style>
