<template>
	<div id="dmx">
		<Section title="Actions" class="info centered">
			<BaseButton @onclicked="fetchData" color="yellow" text="Update"/>
			<BaseButton @onclicked="cycleAutoUpdate" :text="s_updateInterval"/>
			<BaseButton @onclicked="cycleViewType" :text="s_viewType"/>
			<BaseButton @onclicked="testOutput" color="red" text="Test Output"/>
		</Section>

		<UniverseView v-if="$root.$data.config.port_a_enabled == 1" :port="0" :type="$root.$data.config.port_a_type" :universe="$root.$data.config.port_a_universe" :size="$root.$data.config.port_a_numChannels" :data="portAData" :view-type="viewType"/>
		<UniverseView v-if="$root.$data.config.port_b_enabled == 1" :port="1" :type="$root.$data.config.port_b_type" :universe="$root.$data.config.port_b_universe" :size="$root.$data.config.port_b_numChannels" :data="portBData" :view-type="viewType"/>
		<!--<UniverseView universe="11" :data="fakeData()" :view-type="viewType"/>-->
	</div>
</template>

<script>
import Section from '@/components/Section'
import UniverseView from '@/components/UniverseView'

const UNI_LENGTH = 512
const AUTOUPDATE_INTERVALS = [1000, 2000, 5000, 10000, 30000, 60000]

export default {
	name: 'Dmx',
	components: {
		Section,
		UniverseView
	},
	data() {
		return {
			autoUpdate: 2,
			autoUpdateInterval: null,
			viewType: 0,
			portAData: new Uint8Array(),
			portBData: new Uint8Array()
		}
	},
	mounted() {
		this.loadPrefs()
		this.setAutoUpdate()
	},
	activated() {
		this.fetchData()
		this.setAutoUpdate()
	},
	deactivated() {
		this.clearAutoUpdate()
	},
	methods: {
		fetchData() {
			this.$root.fetch('/dmx')
		    	.then(r => {
					if (!r.ok)
						throw new Error("Couldn't fetch DMX data!")
					r.arrayBuffer().then(data => {
						let portAUni = this.$root.$data.config.port_a_universe
						let portBUni = this.$root.$data.config.port_b_universe
						let portAOffset = this.$root.$data.config.port_a_offset
						let portBOffsetBase = this.$root.$data.config.port_b_offset
						let portBOffset = portAUni != portBUni ? UNI_LENGTH + portBOffsetBase : portBOffsetBase
						//console.log(portBOffset)

						let uniLength = Math.min(UNI_LENGTH, data.byteLength)
						this.portAData = new Uint8Array(data, portAOffset, uniLength)
						if (this.$root.$data.config.port_b_enabled > 0)
							this.portBData = new Uint8Array(data, portBOffset, uniLength)
						
						/*console.log(this.portAData)
						console.log(this.portBData)*/
					})
					.catch(e => { throw new Error("Couldn't parse DMX data! (" + e + ")") })
				})
		},
		/*fakeData() {
			let data = []
			for (let i = 0; i < UNI_LENGTH; i++)
				data.push(Math.round(Math.random() * 255))
			return data
		},*/
		setAutoUpdate() {
			this.clearAutoUpdate()
			if (this.autoUpdate > 0)
				this.autoUpdateInterval = setInterval(this.fetchData, AUTOUPDATE_INTERVALS[this.autoUpdate - 1])
		},
		clearAutoUpdate() {
			clearInterval(this.autoUpdateInterval)
			this.autoUpdateInterval = null
		},
		cycleAutoUpdate() {
			this.autoUpdate++
			if (this.autoUpdate > 6)
				this.autoUpdate = 0
			this.setAutoUpdate()
			this.savePrefs()
		},
		cycleViewType() {
			this.viewType++
			if (this.viewType > 2)
				this.viewType = 0
			this.savePrefs()
		},
		savePrefs() {
			this.$root.setPref('dmxView_autoUpdate', this.autoUpdate)
			this.$root.setPref('dmxView_viewType', this.viewType)
		},
		loadPrefs() {
			let _autoUpdate = this.$root.getPref('dmxView_autoUpdate')
			if (_autoUpdate != null)
				this.autoUpdate = _autoUpdate
			let _viewType = this.$root.getPref('dmxView_viewType')
			if (_viewType != null)
				this.viewType = _viewType
		},

		testOutput() {
			let confirmed = confirm('While testing output, all channels on enabled ports will briefly highlight in quick succession.\n\nNOTE: Regular output will stop until testing is completed!\n\nAre you sure?')
			if (confirmed)
				this.$root.fetch('/testoutput')
		}
	},
	computed: {
		s_updateInterval() {
			let s
			if (this.autoUpdate > 0)
				s = (AUTOUPDATE_INTERVALS[this.autoUpdate - 1] / 1000) + 's'
			else
				s = 'Disabled'
			return 'Auto Update [' + s + ']'
		},
		s_viewType() {
			let s
			switch (this.viewType) {
				case 0:
					s = 'Percentage (00-FL)'
					break
				case 1:
					s = 'Decimal (0-255)'
					break
				case 2:
					s = 'Hex (00-FF)'
					break
			}
			return 'DMX View Type [' + s + ']'
		}
	}
}
</script>

<style scoped>

</style>
