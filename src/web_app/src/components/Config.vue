<template>
	<div id="config">
		<SyncLoader :loading="isFetching" color="#c9dce4"></SyncLoader>

		<div v-if="!isReady && !isFetching">
			<Section title="Error" class="info error centered">
				<b>Couldn't fetch settings!</b><br><p>{{$root.$data.messages.ERROR_FETCHING_HINT}}</p>
			</Section>
		</div>
		<div v-if="isReady">
			<Section title="Actions" class="info centered">
				<BaseButton @onclicked="resetConfig" color="red" text="Reset to defaults"/>
				<BaseButton v-if="!editing" @onclicked="setEditing(true)" color="yellow" text="Edit"/>
				<span v-if="editing">
					<BaseButton @onclicked="setEditing(false)" text="Cancel"/>
					<BaseButton @onclicked="saveConfig" color="yellow" text="Save"/>
				</span>
			</Section>
			<Section title="Filters" class="info centered" collapsable>
				<a @click="toggleCats('misc')" class="chip" :class="{ on: isCatVisible('misc') }"><span class="indicator">MISC</span></a>
				<a @click="toggleCats('dmx')" class="chip dmx" :class="{ on: isCatVisible('dmx') }"><span class="indicator">OUTPUT</span></a>
				<a @click="toggleCats('mqtt')" class="chip mqtt" :class="{ on: isCatVisible('mqtt') }"><span class="indicator">MQTT</span></a>
			</Section>

			<Section cat="misc" :visibleCats="visibleCats" title="General" collapsable>
				<!-- name -->
				<Field v-model.trim="$v.config._.name.$model"
					type="input" title="node name"
					:editing="editing"
					:class="valid($v.config._.name)"/>
			</Section>

			<Section cat="misc" :visibleCats="visibleCats" title="WiFi" collapsable>
				<!-- wifi_ssid -->
				<Field v-model.trim="$v.config._.wifi_ssid.$model"
					title="ssid"
					type="input"
					:editing="editing"
					:class="valid($v.config._.wifi_ssid)">
					<div slot="head" v-if="editing">
						<BaseButton @onclicked="fetchNetworks" color="yellow" text="Scan" :spinner="isFetchingNetworks"/>
					</div>
					<div v-if="editing && networks.length > 0" style="display: flex; flex-flow: wrap; gap: 4px; margin: 4px auto">
						<span class="chip" v-for="ssid in networks" @click="setSSID(ssid)">{{ssid}}</span>
					</div>
				</Field>
				<!-- wifi_passphrase -->
				<Field v-model.trim="$v.config._.wifi_passphrase.$model"
					type="input"
					password
					title="password"
					:editing="editing"
					:class="valid($v.config._.wifi_passphrase)"/>
			</Section>

			<Section cat="misc" :visibleCats="visibleCats" title="Networking" class="general" collapsable>
				<!-- mac -->
				<Field readonly
					:value="config._.mac"
					type="input"
					title="mac"
					:editing="editing"/>
				<!-- dhcp -->
				<Field v-model="$v.config._.dhcp.$model"
					type="dropdown"
					:options="['no', 'yes']"
					title="dhcp"
					:editing="editing"
					:class="valid($v.config._.dhcp)"/>
				<!-- ip -->
				<Field v-model="$v.config._.ip.$model"
					:readonly="dhcpEnabled"
					type="input"
					title="ip"
					:editing="editing"
					:class="valid($v.config._.ip)"/>
				<!-- netmask -->
				<Field v-model="$v.config._.netmask.$model"
					:readonly="dhcpEnabled"
					type="input"
					title="netmask"
					:editing="editing"
					:class="valid($v.config._.netmask)"/>
				<!-- gateway -->
				<Field v-model="$v.config._.gateway.$model"
					:readonly="dhcpEnabled"
					type="input"
					title="gateway"
					:editing="editing"
					:class="valid($v.config._.gateway)"/>
			</Section>

			<Section cat="dmx" :visibleCats="visibleCats" title="ArtNet" collapsable>
				<!-- artnet_enabled -->
				<Field v-model="$v.config._.artnet_enabled.$model"
					type="dropdown"
					:options="['no', 'yes']"
					title="enabled"
					:editing="editing"
					:class="valid($v.config._.artnet_enabled)"/>
			</Section>

			<Section cat="dmx" :visibleCats="visibleCats" title="E1.31 (sACN)" collapsable>
				<!-- sacn_enabled -->
				<Field v-model="$v.config._.sacn_enabled.$model"
					type="dropdown"
					:options="['no', 'yes']"
					title="enabled"
					:editing="editing"
					:class="valid($v.config._.sacn_enabled)"/>
			</Section>

			<Section cat="dmx" :visibleCats="visibleCats" title="Output" collapsable>
				<Section cat="info" subtitle="Info">
					<p>Smoothing interpolates values between the last 2 received frames. Set "time" to the known delay (ms) between frames and "threshold" to the max delta between values after which the values will snap instead of interpolating. Setting "time" to 0 disables this feature. This is ideal for when the ArtNet/sACN frequency is less than the output's.</p>
				</Section>
				<Section subtitle="Port A">
					<!-- port_a_enabled -->
					<Field v-model="$v.config._.port_a_enabled.$model"
						type="dropdown"
						:options="['no', 'yes']"
						title="enabled"
						:editing="editing"
						:class="valid($v.config._.port_a_enabled)"/>
					<!-- port_a_type -->
					<Field v-model="$v.config._.port_a_type.$model"
						:readonly="!portAEnabled"
						type="dropdown"
						:options="['DMX', 'PIXELS']"
						title="type"
						:editing="editing"
						:class="valid($v.config._.port_a_type)"/>
					<!-- port_a_rdmEnabled -->
					<Field v-model="$v.config._.port_a_rdmEnabled.$model"
						v-if="portAisDmx"
						:readonly="!portAEnabled"
						type="dropdown"
						:options="['no', 'yes']"
						title="rdm"
						:editing="editing"
						:class="valid($v.config._.port_a_rdmEnabled)"/>
					<!-- port_a_numChannels -->
					<Field v-model="$v.config._.port_a_numChannels.$model"
						:readonly="!portAEnabled"
						type="input"
						:title="config._.port_a_type == 0 ? 'channels' : 'pixels amount'"
						:editing="editing"
						:class="valid($v.config._.port_a_numChannels)"/>
					<!-- port_a_universe -->
					<Field v-model="$v.config._.port_a_universe.$model"
						:readonly="!portAEnabled"
						type="input"
						title="universe"
						:editing="editing"
						:class="valid($v.config._.port_a_universe)"/>
					<!-- port_a_offset -->
					<Field v-model="$v.config._.port_a_offset.$model"
						v-if="!portAisDmx"
						:readonly="!portAEnabled"
						type="input"
						title="address offset"
						:editing="editing"
						:class="valid($v.config._.port_a_offset)"/>
                    <!-- port_a_brightness -->
                    <Field v-model="$v.config._.port_a_brightness.$model"
                        :readonly="!portAEnabled"
                        type="input"
                        title="brightness"
                        :editing="editing"
                        :class="valid($v.config._.port_a_brightness)"/>
					<!-- port_a_smoothtime -->
                    <Field v-model="$v.config._.port_a_smoothtime.$model"
                        :readonly="!portAEnabled"
                        type="input"
                        title="smoothing time"
                        :editing="editing"
                        :class="valid($v.config._.port_a_smoothtime)"/>
					<!-- port_a_smoothingthreshold -->
                    <Field v-model="$v.config._.port_a_smoothingthreshold.$model"
                        :readonly="!portAEnabled"
                        type="input"
                        title="smoothing threshold"
                        :editing="editing"
                        :class="valid($v.config._.port_a_smoothingthreshold)"/>
				</Section>
				<Section subtitle="Port B">
					<!-- port_b_enabled -->
					<Field v-model="$v.config._.port_b_enabled.$model"
						type="dropdown"
						:options="['no', 'yes']"
						title="enabled"
						:editing="editing"
						:class="valid($v.config._.port_b_enabled)"/>
					<!-- port_b_type -->
					<Field v-model="$v.config._.port_b_type.$model"
						:readonly="!portBEnabled"
						type="dropdown"
						:options="['DMX', 'PIXELS']"
						title="type"
						:editing="editing"
						:class="valid($v.config._.port_b_type)"/>
					<!-- port_b_rdmEnabled -->
					<Field v-model="$v.config._.port_b_rdmEnabled.$model"
						v-if="portBisDmx"
						:readonly="!portBEnabled"
						type="dropdown"
						:options="['no', 'yes']"
						title="rdm"
						:editing="editing"
						:class="valid($v.config._.port_b_rdmEnabled)"/>
					<!-- port_b_numChannels -->
					<Field v-model="$v.config._.port_b_numChannels.$model"
						:readonly="!portBEnabled"
						type="input"
						:title="config._.port_b_type == 0 ? 'channels' : 'pixels amount'"
						:editing="editing"
						:class="valid($v.config._.port_b_numChannels)"/>
					<!-- port_b_universe -->
					<Field v-model="$v.config._.port_b_universe.$model"
						:readonly="!portBEnabled"
						type="input"
						title="universe"
						:editing="editing"
						:class="valid($v.config._.port_b_universe)"/>
					<!-- port_b_offset -->
					<Field v-model="$v.config._.port_b_offset.$model"
						v-if="!portBisDmx"
						:readonly="!portBEnabled"
						type="input"
						title="address offset"
						:editing="editing"
						:class="valid($v.config._.port_b_offset)"/>
                    <!-- port_b_brightness -->
                    <Field v-model="$v.config._.port_b_brightness.$model"
                        :readonly="!portBEnabled"
                        type="input"
                        title="brightness"
                        :editing="editing"
                        :class="valid($v.config._.port_b_brightness)"/>
					<!-- port_b_smoothtime -->
                    <Field v-model="$v.config._.port_b_smoothtime.$model"
                        :readonly="!portBEnabled"
                        type="input"
                        title="smoothing time"
                        :editing="editing"
                        :class="valid($v.config._.port_b_smoothtime)"/>
					<!-- port_b_smoothingthreshold -->
                    <Field v-model="$v.config._.port_b_smoothingthreshold.$model"
                        :readonly="!portBEnabled"
                        type="input"
                        title="smoothing threshold"
                        :editing="editing"
                        :class="valid($v.config._.port_b_smoothingthreshold)"/>
				</Section>
			</Section>

			<Section cat="mqtt" :visibleCats="visibleCats" title="MQTT" collapsable>
				<!-- mqtt_enabled -->
				<Field v-model="$v.config._.mqtt_enabled.$model"
					type="dropdown"
					:options="['no', 'yes']"
					title="enabled"
					:editing="editing"
					:class="valid($v.config._.mqtt_enabled)"/>
				<!-- mqtt_broker_address -->
				<Field v-model="$v.config._.mqtt_broker_address.$model"
					:readonly="!mqttEnabled"
					type="input"
					title="broker address"
					:editing="editing"
					:class="valid($v.config._.mqtt_broker_address)"/>
				<!-- mqtt_broker_port -->
				<Field v-model="$v.config._.mqtt_broker_port.$model"
					:readonly="!mqttEnabled"
					type="input"
					title="broker port"
					:editing="editing"
					:class="valid($v.config._.mqtt_broker_port)"/>
				<!-- mqtt_topic_prefix -->
				<Field v-model.trim="$v.config._.mqtt_topic_prefix.$model"
					:readonly="!mqttEnabled"
					type="input"
					title="topic prefix [<a title='Full topic will be prefix/group'>?</a>]"
					:editing="editing"
					:class="valid($v.config._.mqtt_topic_prefix)"/>
				<!-- mqtt_topic_group -->
				<Field v-model.trim="$v.config._.mqtt_topic_group.$model"
					:readonly="!mqttEnabled"
					type="input"
					title="topic group"
					:editing="editing"
					:class="valid($v.config._.mqtt_topic_group)"/>
				<!-- mqtt_dmx_mergemode -->
				<Field v-model="$v.config._.mqtt_dmx_mergemode.$model"
					:readonly="!mqttEnabled"
					type="dropdown"
					:options="['LTP', 'HTP']"
					title="DMX merge mode"
					:editing="editing"
					:class="valid($v.config._.mqtt_dmx_mergemode)"/>
				<!-- mqtt_dmx_fadetime -->
				<Field v-model="$v.config._.mqtt_dmx_fadetime.$model"
					:readonly="!mqttEnabled"
					type="input"
					title="DMX fade time (ms)"
					:editing="editing"
					:class="valid($v.config._.mqtt_dmx_fadetime)"/>

			<Section subtitle="DMX Scenes [<a title='Syntax: [channel]=[value],[...]'>?</a>]">
					<!-- mqtt_dmx_sceneA -->
					<Field v-model.trim="$v.config._.mqtt_dmx_sceneA.$model"
						:readonly="!mqttEnabled"
						type="input"
						title="Scene A"
						:editing="editing"
						:class="valid($v.config._.mqtt_dmx_sceneA)"/>
					<!-- mqtt_dmx_sceneB -->
					<Field v-model.trim="$v.config._.mqtt_dmx_sceneB.$model"
						:readonly="!mqttEnabled"
						type="input"
						title="Scene B"
						:editing="editing"
						:class="valid($v.config._.mqtt_dmx_sceneB)"/>
					<!-- mqtt_dmx_sceneC -->
					<Field v-model.trim="$v.config._.mqtt_dmx_sceneC.$model"
						:readonly="!mqttEnabled"
						type="input"
						title="Scene C"
						:editing="editing"
						:class="valid($v.config._.mqtt_dmx_sceneC)"/>
					<!-- mqtt_dmx_sceneD -->
					<Field v-model.trim="$v.config._.mqtt_dmx_sceneD.$model"
						:readonly="!mqttEnabled"
						type="input"
						title="Scene D"
						:editing="editing"
						:class="valid($v.config._.mqtt_dmx_sceneD)"/>
			</Section>
			</Section>
		</div>
	</div>
</template>

<script>
import Section from '@/components/Section'
import Field from '@/components/ConfigField'

import { required, requiredIf, maxLength, between, alphaNum, numeric, ipAddress } from 'vuelidate/lib/validators'

import SyncLoader from 'vue-spinner/src/SyncLoader'

export default {
	name: 'Config',
	components: {
		Section,
		Field,
		SyncLoader
	},
	data() {
		return {
			visibleCats: ['misc', 'dmx', 'mqtt'],
			editing: false,
			isFetching: false,
			isReady: false,
			config: {},
			networks: {},
			isFetchingNetworks: false
		}
	},
	methods: {
		toggleCats(cat) {
			let i = this.visibleCats.indexOf(cat)
			if (i > -1 && this.visibleCats.length > 1)
				this.visibleCats.splice(i, 1)
			else if (this.visibleCats.indexOf(cat) < 0)
				this.visibleCats.push(cat)

			this.$root.setPref('Config_visibleCats', this.visibleCats)
		},
		isCatVisible(cat) {
			return this.visibleCats.indexOf(cat) > -1
		},
		setEditing(s) {
			if (!s)
				this.cloneConfig()
			this.editing = s
		},
		fetchConfig() {
			this.isFetching = true
			this.isReady = false
	  		this.$root.fetch('/settings')
				.then(r => {
					if (!r.ok) {
						this.isFetching = false
						throw new Error("Couldn't fetch config!")
					}
					r.json()
					.then(c => {
						let keys = Object.keys(this.$root.$data.config)
						for (let keyIdx = 0; keyIdx < keys.length; keyIdx++)
							this.$root.$data.config[keys[keyIdx]] = c[keyIdx]
						this.cloneConfig()
						document.title = this.$root.$data.config.name
						this.isFetching = false
						this.isReady = true
					})
					.catch(e => {
						this.isFetching = false
						throw new Error("Couldn't parse config! (" + e + ")")
					})
				})
		},
		saveConfig() {
			if (this.$v.$invalid)
				return

			let newConfig = JSON.stringify(Object.values(this.config._))
			console.log(newConfig)

			this.$root.fetch('/settings/save', {
				method: 'POST',
				body: newConfig
			}).then(r => {
				if (r.ok)
                    this.$router.go()
					//this.fetchConfig()
			})

			this.$root.$data.config = Object.assign({}, this.config._)
			this.setEditing(false)
		},
		resetConfig() {
			let confirmed = confirm('Are you sure?')
			if (confirmed)
				this.$root.fetch('/settings/reset')
					.then(location.reload())
		},
		cloneConfig() {
			let copy = Object.assign({}, this.$root.$data.config)
			this.$set(this.config, '_', copy)
		},
		valid(v) {
			v.$touch()
			return {
				error: v.$error/*,
				dirty: v.$dirty*/
			}
		},
		fetchNetworks() {
			if (this.isFetchingNetworks)
				return

			this.networks = {}
			this.isFetchingNetworks = true
			this.$root.fetch('/networks')
				.then(r => {
					r.json()
						.then(json => json.sort(function(a, b) { return b.rssi - a.rssi }))
						.then(json => {
							this.networks = json.map(i => i.ssid)
							this.isFetchingNetworks = false
						})
				})
				.catch(e => {
					this.isFetchingNetworks = false
					// The node returns an empty response at first scan due to the async nature of it -- just retry to get the actual results
					this.fetchNetworks()
				})
		},
		setSSID(ssid) {
			this.config._.wifi_ssid = ssid
		}
	},
	created() {
		this.cloneConfig()
		this.fetchConfig()
	},
	mounted() {
		let _storedVisibleCats = this.$root.getPref('Config_visibleCats')
		if (_storedVisibleCats != null)
			this.visibleCats = _storedVisibleCats
	},
	computed: {
		dhcpEnabled() {
			return this.config._.dhcp == 1
		},
		artnetEnabled() {
			return this.config._.artnet_enabled == 1
		},
		sacnEnabled() {
			return this.config._.sacn_enabled == 1
		},
		portAEnabled() {
			return this.config._.port_a_enabled == 1
		},
		portAisDmx() {
			return this.config._.port_a_type == 0
		},
		portBEnabled() {
			return this.config._.port_b_enabled == 1
		},
		portBisDmx() {
			return this.config._.port_b_type == 0
		},
		mqttEnabled() {
			return this.config._.mqtt_enabled == 1
		}
	},
	validations: {
		config: {
			_: {
				name: {
					required,
					maxLength: maxLength(31)
				},

				wifi_ssid: {
					required,
					maxLength: maxLength(31)
				},
				wifi_passphrase: {
					required,
					maxLength: maxLength(63)
				},

				dhcp: {
					required
				},
				ip: {
					required: requiredIf(function() { return !this.dhcpEnabled }),
					ipAddress
				},
				netmask: {
					required: requiredIf(function() { return !this.dhcpEnabled }),
					ipAddress
				},
				gateway: {
					required: requiredIf(function() { return !this.dhcpEnabled }),
					ipAddress
				},

				artnet_enabled: {
					required
				},

				sacn_enabled: {
					required
				},

				port_a_enabled: {
					required
				},
				port_a_type: {
					required
				},
				port_a_rdmEnabled: {
					required: requiredIf(function() { return this.portAEnabled && this.portAisDmx })
				},
				port_a_numChannels: {
					required: requiredIf(function() { return this.portAEnabled }),
					numeric,
					between: between(1, 512)
				},
				port_a_universe: {
					required: requiredIf(function() { return this.portAEnabled }),
					numeric,
					between: between(0, 32767)
				},
				port_a_offset: {
					required: requiredIf(function() { return this.portAEnabled && !this.portAisDmx }),
					numeric,
					between: between(0, 511)
				},
                port_a_brightness: {
                    required: requiredIf(function() { return this.portAEnabled }),
					numeric,
					between: between(0, 255)
                },
				port_a_smoothtime: {
                    required: requiredIf(function() { return this.portAEnabled }),
					numeric,
					between: between(0, 255)
                },
				port_a_smoothingthreshold: {
                    required: requiredIf(function() { return this.portAEnabled }),
					numeric,
					between: between(0, 255)
                },

				port_b_enabled: {
					required
				},
				port_b_type: {
					required
				},
				port_b_rdmEnabled: {
					required: requiredIf(function() { return this.portBEnabled && this.portBisDmx })
				},
				port_b_numChannels: {
					required: requiredIf(function() { return this.portBEnabled }),
					numeric,
					between: between(1, 512)
				},
				port_b_universe: {
					required: requiredIf(function() { return this.portBEnabled }),
					numeric,
					between: between(0, 32767)
				},
				port_b_offset: {
					required: requiredIf(function() { return this.portBEnabled && !this.portBisDmx }),
					numeric,
					between: between(0, 511)
				},
                port_b_brightness: {
                    required: requiredIf(function() { return this.portBEnabled }),
					numeric,
					between: between(0, 255)
                },
				port_b_smoothtime: {
                    required: requiredIf(function() { return this.portBEnabled }),
					numeric,
					between: between(0, 255)
                },
				port_b_smoothingthreshold: {
                    required: requiredIf(function() { return this.portBEnabled }),
					numeric,
					between: between(0, 255)
                },

				mqtt_enabled: {
					required
				},
				mqtt_broker_address: {
					required: requiredIf(function() { return this.mqttEnabled }),
					ipAddress
				},
				mqtt_broker_port: {
					required: requiredIf(function() { return this.mqttEnabled }),
					numeric,
					between: between(1000, 65535)
				},
				mqtt_topic_prefix: {
					required: requiredIf(function() { return this.mqttEnabled }),
					maxLength: maxLength(43)
				},
				mqtt_topic_group: {
					required: requiredIf(function() { return this.mqttEnabled }),
					maxLength: maxLength(15)
				},
				mqtt_dmx_mergemode: {
					required: requiredIf(function() { return this.mqttEnabled }),
				},
				mqtt_dmx_fadetime: {
					required: requiredIf(function() { return this.mqttEnabled }),
					numeric,
					between: between(0, 60000)
				},
				mqtt_dmx_sceneA: {
					maxLength: maxLength(127)
				},
				mqtt_dmx_sceneB: {
					maxLength: maxLength(127)
				},
				mqtt_dmx_sceneC: {
					maxLength: maxLength(127)
				},
				mqtt_dmx_sceneD: {
					maxLength: maxLength(127)
				},
			}
		}
	}
}
</script>
