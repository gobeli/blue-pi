<template>
  <div id="app">
    <button style="margin-top: 1rem" class="button is-primary" v-if="!characteristic" @click="connect()">
      <span style="margin-right: 0.5rem">Connect</span>
      <i class="fa fa-bluetooth-b"></i>
    </button>

    <div id="regularKeyboard" v-if="characteristic">
      <template v-for="(note, index) in notes">
        <div class="key"
          :class="{active: note === activeNote}" :key="note.note"
          v-on:touchstart="play(note)" v-on:mousedown="play(note)"
          v-if="!note.note.includes('S')"
        >
          {{note.note}} ({{note.key}})
        </div>
        <div class="innerKey"
          :class="{active: note === activeNote}" :key="note.note"
          v-on:touchstart="play(note)" v-on:mousedown="play(note)"
          v-if="note.note.includes('S')"
        >
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
    window.addEventListener('keydown', this.playKey.bind(this), false);
    window.addEventListener('keyup', this.stopKey.bind(this), false);
    window.addEventListener('mouseup', this.stopKey.bind(this), false);
    window.addEventListener('touchend', this.stopKey.bind(this), false);
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
    stopKey() {
      this.down = false;
      clearInterval(this.interval);
      this.activeNote = null;
    },
    playKey(event) {
      const note = this.notes.find(n => n.keyCode === event.keyCode);
      if (note) {
        this.play(note);
      }
    },
    play(note) {
      if (this.down) return;
      this.down = true;
      this.activeNote = note;
      const write = () => {
        this.characteristic.writeValue(
          Uint8Array.from(note.frequency.toString(), x => x.charCodeAt(0))
        );
      };
      write();
      this.interval = setInterval(write, 200);
    },
  },
  data() {
    return {
      characteristic: false,
      notes,
      activeNote: null,
      down: false,
      interval: null
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
