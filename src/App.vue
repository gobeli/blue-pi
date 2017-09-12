<template>
  <div id="app">
    <button style="margin-top: 1rem" class="button is-primary" v-if="!characteristic" @click="connect()">
      <span style="margin-right: 0.5rem">Connect</span>
      <i class="fa fa-bluetooth-b"></i>
    </button>
    <div class="keys" v-if="characteristic">
      <button class="key" v-for="note in notes" :key="note" :class="{ black: note.includes('S'), white: !note.includes('S') }"  @click="play(note)">{{note}}</button>
    </div>
  </div>
</template>

<script>
export default {
  name: 'app',
  methods: {
    connect() {
      // filters: [{services: [0x1234]}]
      navigator.bluetooth.requestDevice({ acceptAllDevices: true, optionalServices: [0x1234] })
        .then(device => device.gatt.connect())
        .then(server => server.getPrimaryService(0x1234))
        .then(service => service.getCharacteristic(0x2345))
        .then((characteristic) => {
          this.characteristic = characteristic;
        })
        .catch(error => console.error(error));
    },
    play(note) {
      const frequency = this.frequencies[this.notes.indexOf(note)];
      console.log(frequency);
      this.characteristic.writeValue(Uint8Array.from(frequency.toString(), x => x.charCodeAt(0)));
    },
  },
  data() {
    return {
      characteristic: false,
      notes:
        ['C4', 'CS4', 'D4', 'DS4', 'E4', 'F4', 'FS4', 'G4', 'GS4', 'A4', 'AS4', 'B4', 'C5', 'CS5', 'D5', 'DS5', 'E5', 'F5'],
      frequencies:
        [262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698],
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
