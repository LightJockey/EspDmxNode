<template>
  <Section :title="title" class="uni-view dmx" collapsable>
    <Section subtitle="Couldn't fetch DMX data!" v-if="!hasValidData" />
    <div v-if="hasValidData" v-html="buildChannelList()" class="channel-list"></div>
  </Section>
</template>

<script>
import Section from '@/components/Section'

const UNI_LENGTH = 512

export default {
  name: 'UniverseView',
  components: {
    Section
  },
  props: {
    port: 0,
    type: 0,
    universe: {},
    size: 0,
    data: Uint8Array,
    viewType: Number
  },
  computed: {
    title() {
      return 'Port ' + this.portToLetter(this.port) + ' (' + (this.type == 0 ? 'DMX' : 'PIXELS') + ') [Universe ' + this.universe + ']'
    },
    hasValidData() {
      return this.data.length > 0 && this.size > 0
    }
  },
  methods: {
    buildChannelList() {
      let html = ''
      // DMX Channels
      if (this.type == 0) {
        for (let i = 0; i < this.size; i++)
          html += '<span class="channel"><b>' + (i + 1) + '</b><i style="background: ' + this.channelBackground(i) + '">' + this.channelValue(i) + '</i></span>'
      }
      // Pixels
      else {
        for (let pi = 0, di = 0; pi < this.size; pi++, di += 3)
          html += '<span class="channel pixel"><b>' + (pi + 1) + '</b><i style="background: ' + this.channelBackground(di) + '"></i></span>'
      }
      return html
    },
    channelPercentage(i) {
      return Math.round(this.data[i] * 100 / 255)
    },
    channelBackground(i) {
      // DMX Channel
      if (this.type == 0) {
        let vp = this.channelPercentage(i)
        return !isNaN(vp) ? 'rgba(222, 212, 234,' + (vp / 100) + ')' : 'rgb(210, 210, 210)'
      }
      // Pixel
      else {
        let vr = this.data[i]
        let vg = this.data[i + 1]
        let vb = this.data[i + 2]
        return 'rgba(' + vr + ',' + vg + ',' + vb + ',1)'
      }
    },
    channelValue(i) {
      let v = this.data[i]
      let fv = ''
      if (!isNaN(v)) {
        switch (this.viewType) {
          case 0: // Percentage
            let vp = this.channelPercentage(i)
            if (vp && vp > 0) {
              if (vp < 10)
                fv += '0' + vp
              else
                fv += vp > 99 ? 'FL' : vp
            }
            else
              fv += '00'
            break
        case 1: // Decimal
          fv += v ? v : 0
          break
        case 2: // Hex
          let vh = v ? v.toString(16).toUpperCase() : '00'
          fv += vh.length == 1 ? vh + vh : vh
          break
        }
      }
      else
        fv = '&nbsp;'
      return fv
    },
    portToLetter(uni) {
      if (uni >= 0 && uni < 26) {
        return String.fromCharCode(97 + uni)
      }
      else
        return ''
    }
  }
}
</script>

<style scoped>
.channel-list {
  margin: 8px 8px;
  /*text-align: center;*/
}

/deep/ .channel {
  display: inline-block;
  /*width: 16px;
  padding: 2px 6px;
  line-height: 18px;*/
  margin-right: 1px;
  text-align: center;
}
/deep/ .channel b {
  font: normal 8px Arial;
  /*font-size: 9px;
  font-weight: normal;*/
  color: rgb(100, 100, 100);
  background: white;
  display: block;
  line-height: 12px;
}
/deep/ .channel i {
  font: normal 10px Arial;
  /*font-size: 10px;
  font-style: normal;*/
  color: rgb(60, 60, 60);
  background: rgb(210, 210, 210);
  display: block;
  width: 20px;
  padding: 4px 0;
  line-height: 12px;
}
/deep/ .channel.pixel i {
  height: 12px;
  border: 1px solid black;
}
</style>
