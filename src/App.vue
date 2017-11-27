<template>
  <div id="app">
    <button style="margin-top: 1rem" class="button is-primary" v-if="!characteristic" @click="connect()">
      <span style="margin-right: 0.5rem">Connect</span>
      <i class="fa fa-bluetooth-b"></i>
    </button>

    <div id="regularKeyboard" v-if="characteristic">
      <template v-for="(note, index) in notes">
        <div class="key" :key="note.note" @click="play(note)" v-if="!note.note.includes('S')">
          {{note.note}} ({{note.key}})
        </div>
        <div class="innerKey" :key="note.note" @click="play(note)"  v-if="note.note.includes('S')">
            <div class="blackkey">
              {{note.note}} ({{note.key}})
            </div>
        </div>
      </template>
    </div>
  </div>
</template>

<script>
import notes from './notes';

export default {
  name: 'app',
  created() {
    window.addEventListener('keydown', this.playKey.bind(this));
  },
  methods: {
    connect() {
      // filters: [{services: [0x1234]}]
      navigator.bluetooth.requestDevice({
        filters: [{ name: 'Blue-Pi' }],
        optionalServices: [0x1234],
      })
        .then(device => device.gatt.connect())
        .then(server => server.getPrimaryService(0x1234))
        .then(service => service.getCharacteristic(0x2345))
        .then((characteristic) => {
          this.characteristic = characteristic;
        })
        .catch(error => console.error(error));
    },
    playKey(event) {
      const note = this.notes.find(n => n.keyCode === event.keyCode);
      if (note) {
        console.log(note);
        this.play(note);
      }
    },
    play(note) {
      this.characteristic.writeValue(
        Uint8Array.from(note.frequency.toString(), x => x.charCodeAt(0))
      );
    },
  },
  data() {
    return {
      characteristic: false,
      notes,
    };
  },
};
</script>

<style lang="scss">
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}

</style>
