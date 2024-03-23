import Vue from 'vue'
import Vuelidate from 'vuelidate'

import App from './App'
import router from './router'

import '@/components/_globals'

Vue.use(Vuelidate)
Vue.config.productionTip = false

/* eslint-disable no-new */
new Vue({
  el: '#app',
  router,
  render: h => h(App),
  data: {
    baseUrl: '',
    messages: {
      ERROR_FETCHING_HINT: "Please check your connection to the node is working."
    },
  	config: {
  	  name: "EspDmxNode",
  	  swVersion: "1",

  	  wifi_ssid: "",
  	  wifi_passphrase: "",

  	  mac: "",
  	  dhcp: 0,
  	  ip: "0.0.0.0",
  	  netmask: "",
      gateway: "",

  	  artnet_enabled: 0,

  	  sacn_enabled: 0,

      //ports: [],

  	  port_a_enabled: 0,
      port_a_type: 0,
  	  port_a_rdmEnabled: 0,
      port_a_numChannels: 512,
  	  port_a_universe: 0,
      port_a_offset: 0,
      port_a_brightness: 255,
      port_a_smoothtime: 0,
      port_a_smoothingthreshold: 25,

  	  port_b_enabled: 0,
      port_b_type: 0,
  	  port_b_rdmEnabled: 0,
      port_b_numChannels: 512,
  	  port_b_universe: 0,
      port_b_offset: 0,
      port_b_brightness: 255,
      port_b_smoothtime: 0,
      port_b_smoothingthreshold: 25,

      mqtt_enabled: 0,
      mqtt_broker_address: "",
      mqtt_broker_port: 1886,
      mqtt_topic_prefix: "",
      mqtt_topic_group: "",
      mqtt_dmx_mergemode: 0,
      mqtt_dmx_fadetime: 0,
      mqtt_dmx_sceneA: "",
      mqtt_dmx_sceneB: "",
      mqtt_dmx_sceneC: "",
      mqtt_dmx_sceneD: ""
  	}/*,
    config_port_obj: {
      enabled: 0,
      type: 0,
      rdmEnabled: 0,
      numChannels: 512,
      universe: 0,
      offset: 0,
      brightness: 255,
      smoothTime: 0,
      smoothingThreshold: 25
    }*/
  },
  methods: {
    fetch(url, options) {
      return fetch(this.baseUrl + url, options)
    },
    setPref(k, v) {
      localStorage.setItem(k, JSON.stringify(v))
    },
    getPref(k) {
      let p = localStorage.getItem(k)
      if (p != null)
        return JSON.parse(p)
      else
        return null
    },
    delPref(k) {
      localStorage.removeItem(k)
    }
  }
})
