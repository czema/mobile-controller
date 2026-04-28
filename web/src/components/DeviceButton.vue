<script setup lang="ts">
import { ref, computed, watch } from 'vue'
import type { Device } from '../types'

const props = defineProps<{
  device: Device
  canvasWidth: number
  canvasHeight: number
  gridCellW: number
  gridCellH: number
  iconSize: number
}>()

const emit = defineEmits<{
  tap: [device: Device]
  longpress: [device: Device]
  moved: [id: string, x: number, y: number]
}>()

const COLS = 8
const ROWS = 6
const MAX_COL = COLS - 2   // reserve last column for FABs
const MAX_ROW = ROWS - 2   // reserve last row for FABs
const LONG_PRESS_MS = 1500
const DRAG_THRESHOLD = 8

const displayX = ref(props.device.x)
const displayY = ref(props.device.y)
const isDragging = ref(false)
const isPressed = ref(false)
const isPendingLongPress = ref(false)

watch(
  () => [props.device.x, props.device.y] as [number, number],
  ([x, y]) => {
    if (!isDragging.value) {
      displayX.value = x
      displayY.value = y
    }
  }
)

let longPressTimer = 0
let pulseTimer = 0
let startClientX = 0
let startClientY = 0
let startDevX = 0
let startDevY = 0
let hasMoved = false

const clamp = (v: number, lo: number, hi: number) => Math.max(lo, Math.min(v, hi))

const snapX = (px: number) => {
  const col = Math.min(Math.max(0, Math.round(px / props.gridCellW)), MAX_COL)
  return col * props.gridCellW
}

const snapY = (py: number) => {
  const row = Math.min(Math.max(0, Math.round(py / props.gridCellH)), MAX_ROW)
  return row * props.gridCellH
}

const formatTime = (iso: string): string => {
  const d = new Date(iso)
  const now = new Date()
  if (d.toDateString() !== now.toDateString()) return ''
  return d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
}

const onPointerDown = (e: PointerEvent) => {
  e.preventDefault()
  ;(e.currentTarget as HTMLElement).setPointerCapture(e.pointerId)

  isPressed.value = true
  hasMoved = false
  isDragging.value = false
  isPendingLongPress.value = false
  startClientX = e.clientX
  startClientY = e.clientY
  startDevX = props.device.x
  startDevY = props.device.y

  pulseTimer = window.setTimeout(() => {
    if (isPressed.value && !hasMoved) isPendingLongPress.value = true
  }, 600)

  longPressTimer = window.setTimeout(() => {
    if (!hasMoved) {
      isPendingLongPress.value = false
      isPressed.value = false
      emit('longpress', props.device)
    }
  }, LONG_PRESS_MS)
}

const onPointerMove = (e: PointerEvent) => {
  if (!isPressed.value) return

  const dx = e.clientX - startClientX
  const dy = e.clientY - startClientY

  if (!hasMoved && Math.hypot(dx, dy) > DRAG_THRESHOLD) {
    hasMoved = true
    isDragging.value = true
    isPendingLongPress.value = false
    window.clearTimeout(longPressTimer)
    window.clearTimeout(pulseTimer)
  }

  if (isDragging.value) {
    const rawX = clamp(startDevX + dx, 0, props.canvasWidth - props.gridCellW)
    const rawY = clamp(startDevY + dy, 0, props.canvasHeight - props.gridCellH)
    displayX.value = snapX(rawX)
    displayY.value = snapY(rawY)
  }
}

const onPointerUp = () => {
  window.clearTimeout(longPressTimer)
  window.clearTimeout(pulseTimer)

  if (isDragging.value) {
    emit('moved', props.device.id, displayX.value, displayY.value)
  } else if (isPressed.value) {
    emit('tap', props.device)
  }

  isPressed.value = false
  isDragging.value = false
  isPendingLongPress.value = false
}

const bodySize = computed(() => `${props.iconSize}px`)
const iconFontSize = computed(() => `${Math.round(props.iconSize * 0.38)}px`)

const style = computed(() => ({
  left: `${displayX.value}px`,
  top: `${displayY.value}px`,
  width: `${props.gridCellW}px`,
  height: `${props.gridCellH}px`,
}))
</script>

<template>
  <div
    class="dev"
    :class="{
      [`dev--${device.type}`]: true,
      'dev--pressed': isPressed && !isDragging,
      'dev--dragging': isDragging,
      'dev--pending': isPendingLongPress,
    }"
    :style="style"
    @pointerdown="onPointerDown"
    @pointermove="onPointerMove"
    @pointerup="onPointerUp"
    @pointercancel="onPointerUp"
  >
    <div class="dev__body" :style="{ width: bodySize, height: bodySize }">
      <font-awesome-icon :icon="device.iconName" class="dev__icon" :style="{ fontSize: iconFontSize }" />
    </div>
    <span class="dev__label">{{ device.name }}</span>
    <span v-if="device.lastSent && formatTime(device.lastSent)" class="dev__time">
      {{ formatTime(device.lastSent) }}
    </span>
  </div>
</template>

<style scoped>
.dev {
  position: absolute;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 5px;
  cursor: pointer;
  user-select: none;
  touch-action: none;
  -webkit-touch-callout: none;
  transition: opacity 0.1s;
}

.dev__body {
  border-radius: 22%;
  display: flex;
  align-items: center;
  justify-content: center;
  background: radial-gradient(ellipse at 38% 28%, #50506a 0%, #1c1c2c 100%);
  border: 1px solid rgba(255, 255, 255, 0.07);
  box-shadow:
    0 5px 14px rgba(0, 0, 0, 0.85),
    0 2px 4px rgba(0, 0, 0, 0.6),
    inset 0 1px 0 rgba(255, 255, 255, 0.14),
    inset 0 -2px 5px rgba(0, 0, 0, 0.5);
  transition: transform 0.1s ease, box-shadow 0.1s ease;
  will-change: transform, box-shadow;
  flex-shrink: 0;
}

.dev--ir .dev__body {
  box-shadow:
    0 5px 14px rgba(0, 0, 0, 0.85),
    0 2px 4px rgba(0, 0, 0, 0.6),
    0 0 0 2px rgba(0, 178, 216, 0.2),
    inset 0 1px 0 rgba(255, 255, 255, 0.14),
    inset 0 -2px 5px rgba(0, 0, 0, 0.5);
}

.dev--rf .dev__body {
  box-shadow:
    0 5px 14px rgba(0, 0, 0, 0.85),
    0 2px 4px rgba(0, 0, 0, 0.6),
    0 0 0 2px rgba(244, 162, 97, 0.2),
    inset 0 1px 0 rgba(255, 255, 255, 0.14),
    inset 0 -2px 5px rgba(0, 0, 0, 0.5);
}

.dev__icon {
  color: #c0c0d4;
}

.dev--ir .dev__icon { color: #7de0f4; }
.dev--rf .dev__icon { color: #f4cc8a; }

.dev__label {
  font-size: 0.7rem;
  font-weight: 500;
  color: #c0c0d4;
  letter-spacing: 0.3px;
  text-align: center;
  width: 100%;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  line-height: 1.2;
  padding: 0 4px;
}

.dev__time {
  font-size: 0.62rem;
  color: #8080a0;
  text-align: center;
  line-height: 1;
}

.dev--pressed .dev__body {
  transform: scale(0.9) translateY(2px);
  box-shadow:
    0 1px 4px rgba(0, 0, 0, 0.95),
    inset 0 3px 8px rgba(0, 0, 0, 0.7),
    inset 0 1px 2px rgba(0, 0, 0, 0.4);
}

.dev--dragging {
  opacity: 0.9;
  z-index: 999;
}

.dev--dragging .dev__body {
  transform: scale(1.12);
  box-shadow:
    0 16px 40px rgba(0, 0, 0, 0.95),
    0 4px 12px rgba(0, 0, 0, 0.7),
    inset 0 1px 0 rgba(255, 255, 255, 0.15);
}

@keyframes lp-pulse {
  0%   { box-shadow: 0 5px 14px rgba(0,0,0,0.85), 0 0 0 0   rgba(255,255,255,0.45); }
  100% { box-shadow: 0 5px 14px rgba(0,0,0,0.85), 0 0 0 24px rgba(255,255,255,0); }
}

.dev--pending .dev__body {
  animation: lp-pulse 0.65s ease-out infinite;
}
</style>
