<template>
	<div id="status">
		<SyncLoader :loading="isFetching" color="#c9dce4"></SyncLoader>

		<div v-if="status.length > 0">
			<Section title="Actions" class="info centered">
				<BaseButton @onclicked="restartNode" color="red" text="RESTART NODE"/>
			</Section>

			<Section title="Node Info" class="sysinfo info">
				<p>Firmware version: {{status[0]}}</p>
				<p>Build date: {{status[1]}}</p>
				<p>Uptime: {{uptime}}</p>
				<p>Free heap: <span id="heapbar" :class="{ 'g': freeHeap < 80, 'r': freeHeap >= 80 }">{{status[3]}} bytes ({{freeHeap}}% used)</span></p>
			</Section>

			<Section id="logs" title="Last Log Entries" class="sysinfo info">
				<p v-for="entry in logs" v-html="formatLogEntry(entry)"></p>
			</Section>

			<Section title="Firmware Update" class="info centered">
				<p><input @change="handleUpdateFile" type="file" accept=".bin" ref="file"></p>
				<p><BaseButton @onclicked="updateFirmware()" text="Update"/></p>
			</Section>
		</div>
		<div v-else>
			<Section title="Message" class="info centered">
				<b>Couldn't fetch status!</b><br><p>{{$root.$data.messages.ERROR_FETCHING_HINT}}</p>
			</Section>
		</div>
	</div>
</template>

<script>
import Section from '@/components/Section'
import SyncLoader from 'vue-spinner/src/SyncLoader'

export default {
  name: 'Status',
  components: {
  	Section,
  	SyncLoader
  },
  data() {
  	return {
  		autoUpdateInterval: null,
  		isFetching: false,
  		status: [],
  		logs: [],
		updateFile: null
  	}
  },
  methods: {
  	fetch() {
  		this.isFetching = true
  		this.fetchStatus()
  		this.fetchLogs()
  		this.isFetching = false
  	},
	fetchStatus() {
  		fetch('/status')
  			.then(r => {
				if (r.ok) {
	  				r.json()
			    	.then(c => this.status = c)
	    		}
    		})
	},
	fetchLogs() {
  		fetch('/logs')
	    	.then(r => {
    			if (r.ok) {
		    		r.text()
					.then(text => this.logs = text.split('\n'))
    				//.then(text => this.logs = text.split(/\r|\n/))
    			}
			})
	},
	setAutoUpdate() {
		this.clearAutoUpdate()
		this.autoUpdateInterval = setInterval(this.fetch, 10000)
	},
	clearAutoUpdate() {
		clearInterval(this.autoUpdateInterval)
		this.autoUpdateInterval = null
	},
	formatLogEntry(e) {
		let lt = e.substring(0, 7)
		let lm = e.substring(7, e.length)

		let s = ''
		if (lt.includes("DEBG"))
			s += "<span class='debug'>debug</span>"
		else if (lt.includes("INFO"))
			s += "<span class='info'>info</span>"
		else if (lt.includes("ERRO"))
			s += "<span class='error'>error</span>"
		else if (lt.includes("OTA"))
			s += "<span class='ota'>ota</span>"
		else if (lt.includes("CONF"))
			s += "<span class='config'>config</span>"
		else if (lt.includes("WIFI"))
			s += "<span class='wifi'>wifi</span>"
		else if (lt.includes("ARTN"))
			s += "<span class='artnet'>artnet</span>"
		else if (lt.includes("E131"))
			s += "<span class='e131'>e131</span>"
		else if (lt.includes("OUTP"))
			s += "<span class='output'>output</span>"
		else if (lt.includes("MQTT"))
			s += "<span class='mqtt'>mqtt</span>"
		s += lm

		return s
	},

	restartNode() {
		let confirmed = confirm('Are you sure?')
		if (confirmed)
			fetch('/restart')
				.then(r => location.reload())
	},

	handleUpdateFile() {
		this.updateFile = this.$refs.file.files[0]
	},
	updateFirmware() {
		if (!this.updateFile) {
			alert('No file selected!')
			return
		}
		let fd = new FormData()
		fd.append('file', this.updateFile)
		
		fetch('/update', {
			method: 'POST',
			headers: {
				'Auth': 'dmxnodeota'
			},
			body: fd
		}).then(r => r.text()).then(r => {
			this.fetchLogs()
			alert(r)
		})
	}
  },
  computed: {
  	uptime() {
  		let ts = Number(this.status[2])
  		let d = Math.floor(ts / (3600*24))
  		let h = Math.floor(ts % (3600*24) / 3600)
  		let m = Math.floor(ts % 3600 / 60)
  		let s = Math.floor(ts % 60)

  		let dDisplay = d + (d == 1 ? " day, " : " days, ")
  		let hDisplay = h + (h == 1 ? " hour, " : " hours, ")
  		let mDisplay = m + (m == 1 ? " minute, " : " minutes, ")
  		let sDisplay = s + (s == 1 ? " second" : " seconds")
  		return dDisplay + hDisplay + mDisplay + sDisplay
  	},
  	freeHeap() {
  		if (!isNaN(this.status[3]))
  			return Math.round(((86400 - this.status[3]) / 86400) * 100)
  		else
  			return 0
  	}
  },
  mounted() {
  	this.setAutoUpdate()
  },
  activated() {
	this.fetch()
    this.setAutoUpdate()
  },
  deactivated() {
    this.clearAutoUpdate()
  }
}
</script>

<style scoped>
/deep/ .sysinfo .content > p {
	font-size: .8em;
	background: rgb(230, 230, 230);
	margin: 4px 0;
	padding: 2px 4px;
}
#heapbar {
	color: white;
	background: var(--c-neutral);
	display: inline-block;
	padding: 0 10px;
	text-align: center;
}
#heapbar.g { background: rgb(60, 220, 140); }
#heapbar.r { background: var(--c-error); }

/deep/ #logs .content > p { padding: 0; }
/deep/ #logs .content > p > span {
	background: var(--c-misc);
	display: inline-block;
	padding: 2px 8px;
	text-transform: uppercase;
}
/deep/ #logs .content > p > span.debug { background: var(--c-neutral); }
/deep/ #logs .content > p > span.info { background: var(--c-misc); }
/deep/ #logs .content > p > span.error { background: var(--c-error); }
/deep/ #logs .content > p > span.ota { background: var(--c-neutral); }
/deep/ #logs .content > p > span.config { background: var(--c-misc); }
/deep/ #logs .content > p > span.wifi { background: var(--c-misc); }
/deep/ #logs .content > p > span.artnet { background: var(--c-dmx); }
/deep/ #logs .content > p > span.e131 { background: var(--c-dmx); }
/deep/ #logs .content > p > span.output { background: var(--c-dmx); }
/deep/ #logs .content > p > span.mqtt { background: var(--c-mqtt); }
</style>
