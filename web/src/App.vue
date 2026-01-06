<script setup lang="ts">
  import 'bootstrap/dist/css/bootstrap.min.css';
  import 'bootstrap/dist/js/bootstrap.bundle.min.js'; // Includes Popper.js

  import { ref } from 'vue'

  const config = JSON.parse(window.localStorage.getItem('config') ?? '[]')

  const config_dialog = ref<HTMLDialogElement | null>(null)
  
  const DEVICE_ID = "33003d000947373034353237";
  const TYPES = ["ir", "rf"];
  const INDEXES = [...Array(64).keys()];

  const clicked = async (func: string, arg: string) => {
    const resp = await fetch(`https://api.particle.io/v1/devices/${DEVICE_ID}/${func}`, {
      method: "POST",
      body: new URLSearchParams({
        arg: arg
      }),
      headers: {
        "Authorization": "Bearer 1101b3141ce87335a29da5fea671960bf9a4171c"
      }
    });

    console.log(resp);
  }

  const target = ref<any>(null);
  let timer = 0;

  const addItem = () => {
    target.value = {
      isnew: true,
      label: '',
      type: null,
      index: null
    }

    configure();
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
        "Authorization": "Bearer 1101b3141ce87335a29da5fea671960bf9a4171c"
      }
    });
  }

  const closeConfigure = () => {
    if (!target.value) return;
    delete target.value.handled;

    if (target.value.isnew) {
      delete target.value.isnew
      config.push(target.value)
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
  <dialog ref="config_dialog">
    {{target}}
    <table class="table" v-if="target">
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
          <select id="type" class="form-control" v-model="target.type">
            <option v-for="type in TYPES" :value="type">
              {{ type }}
            </option>
          </select>
        </td>
      </tr>
      <tr>
        <td>
          <label for="index">Index</label>
        </td>
        <td>
          <select id="index" class="form-control" v-model="target.index">
            <option v-for="index in INDEXES" :value="index">
              {{ index + 1 }}
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
        <td>
          <button class="btn btn-danger" @click="program()">Program</button>
        </td>
        <td>
          <button class="btn btn-info" @click="closeConfigure()">Close</button>
        </td>
      </tr>
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
