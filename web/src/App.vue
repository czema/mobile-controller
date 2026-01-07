<script setup lang="ts">
  import 'bootstrap/dist/css/bootstrap.min.css';
  import 'bootstrap/dist/js/bootstrap.bundle.min.js'; // Includes Popper.js

  import { ref, toRaw } from 'vue'

  const config = JSON.parse(window.localStorage.getItem('config') ?? '[]')

  const config_dialog = ref<HTMLDialogElement | null>(null)
  
  const DEVICE_ID = "33003d000947373034353237";
  const PARTICLE_ACCESS_TOKEN = "";
  const TYPES = ["ir", "rf"];
  const INDEXES = [...Array(64).keys()];
  const SLOT_TEMPLATE = {
    is_new: true,
    room: null,
    label: '',
    type: null,
    index: null
  }

  const clicked = async (func: string, arg: string) => {
    const resp = await fetch(`https://api.particle.io/v1/devices/${DEVICE_ID}/${func}`, {
      method: "POST",
      body: new URLSearchParams({
        arg: arg
      }),
      headers: {
        "Authorization": `Bearer ${PARTICLE_ACCESS_TOKEN}`
      }
    });

    console.log(resp);
  }

  const target = ref<any>(null);
  let timer = 0;

  const addItem = () => {
    const obj = structuredClone(SLOT_TEMPLATE)
    target.value = obj

    configure();
  }

  const getHighestIndex = (type: string) => {
    // Get the highest in-use index with the selected type.
    let highest = -1;
    config.filter((c: any) => c.type === type).forEach((c: any) => {
      if (c.index > highest) {
        highest = c.index;
      }
    })

    return highest;
  }

  const deleteItem = (obj: any) => {
    const idx = config.indexOf(toRaw(obj));
    if (idx !== -1) {
      config.splice(idx, 1);

      // Commit to storage.
      const config_str = JSON.stringify(config)
      window.localStorage.setItem('config', config_str)

      target.value = null;
      window.clearTimeout(timer);
      config_dialog.value!.close();
    }
  }

  const pointerdown = (item: any) => {
    target.value = item;

    timer = window.setTimeout(configure, 500);
  }

  const pointerup = () => {
    if (!target.value || target.value.handled) return;
    window.clearTimeout(timer);

    clicked(`${target.value.type}_send`, target.value.index)
  }

  const configure = () => {
    if (!target.value) return;
    target.value.handled = true;

    config_dialog.value!.showModal();
  }

  const program = async () => {
    if (!target.value) return;

    const func = `${target.value.type}_recv`;

    const resp = await fetch(`https://api.particle.io/v1/devices/${DEVICE_ID}/${func}`, {
      method: "POST",
      body: new URLSearchParams({
        arg: target.value.index
      }),
      headers: {
        "Authorization": `Bearer ${PARTICLE_ACCESS_TOKEN}`
      }
    });
  }

  const closeConfigure = () => {
    if (!target.value) return;
    delete target.value.handled;

    if (target.value.is_new) {
      // Do not create the item if label isn't set.
      if (target.value.label) {
        delete target.value.is_new
        config.push(toRaw(target.value))
      }
    }

    // Commit to storage.
    const config_str = JSON.stringify(config)
    window.localStorage.setItem('config', config_str)

    target.value = null;
    window.clearTimeout(timer);
    config_dialog.value!.close();
  }

</script>

<template>
  <dialog ref="config_dialog" style="margin:auto;box-shadow:rgba(0, 0, 0, 0.3) 4px 4px 10px;border-radius:15px;border:2px solid #1B80B9">
    {{target}}
    <table class="table" v-if="target">
      <tbody>
        <tr>
          <td>
            <label for="label">Label</label>
          </td>
          <td>
            <input type="text" class="form-control" id="label" v-model="target.label" />
          </td>
        </tr>
        <tr>
          <td>
            <label for="type">Type</label>
          </td>
          <td>
            <select id="type" class="form-control" v-model="target.type" @change="event => target.index = getHighestIndex((event!.target as any).value) + 1">
              <option disabled :value="null">Select Type</option>
              <option v-for="type in TYPES" :value="type">
                {{ type }}
              </option>
            </select>
          </td>
        </tr>
        <tr>
          <td>
            <label for="icon">Icon</label>
          </td>
          <td>
          </td>
        </tr>
        <tr>
          <td colspan="2">
            <div style="display:flex;justify-content:space-between">
              <button class="btn btn-warning" @click="program()">Program</button>
              <button class="btn btn-info" @click="closeConfigure()">Close</button>
              <button class="btn btn-danger" @click="deleteItem(target)">Delete</button>
            </div>
          </td>
        </tr>
      </tbody>
    </table>
  </dialog>

  <main>
    <div class="container-fluid">
      <div class="row">
        <div class="col" v-for="item in config">
          <button @pointerdown="pointerdown(item)" @pointerup="pointerup()">{{ item.label ?? "RF 1" }}</button>
        </div>
        <div class="col">
          <button @click="addItem()">Add</button>
        </div>
      </div>
    </div>
  </main>
</template>

<style scoped>
  
</style>
