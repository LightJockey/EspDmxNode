<template>
	<div id="nav">
		<router-link v-bind:to="'/'">Settings</router-link>
		<router-link v-bind:to="'/status'">Status</router-link>
		<router-link v-bind:to="'/output'" class="dmx" :class="{ 'on anim': outputEnabled, 'disabled': !outputEnabled }">
			<span class="label">Output</span>
		</router-link>
		<!--<router-link v-bind:to="'/rdm'" class="dmx" :class="{ on: rdmEnabled }">
			<span class="label">RDM</span>
		</router-link>-->
		<slot name="context-actions"></slot>
	</div>
</template>

<script>
export default {
  name: 'Nav',
  computed: {
  	config() {
  		return this.$root.$data.config
  	},
  	outputEnabled() {
  		return this.config.port_a_enabled == 1 || this.config.port_b_enabled == 1
  	}/*,
  	rdmEnabled() {
  		return (this.config.port_a_enabled && this.config.port_a_rdm) || (this.config.port_b_enabled && this.config.port_b_rdm)
  	}*/
  }
}
</script>

<style scoped>
#nav {
	/*margin: 0 0 25px;*/
	padding: 0 2%;
	display: flex;
	column-gap: .5%;
}
a {
	font-size: 1.1em;
	font-kerning: none;
	color: white;
	background: var(--c-neutral);
	margin-top: 4px;
	padding: 6px 20px 4px;
	box-shadow: inset 0 -10px 8px -8px rgb(180, 180, 180);
	text-decoration: none;
	text-transform: uppercase;
	/*border-radius: 6px 6px 0 0;*/
	border-bottom: 6px solid var(--c-misc);
	flex-grow: 1;
	text-align: center;
	user-select: none;
}
a:hover:not(.disabled), a.router-link-exact-active {
	background: var(--c-misc);
	box-shadow: none;
}
a.disabled {
	pointer-events: none;
	cursor: unset;
}
a.dmx { border-color: var(--c-dmx); }
a.dmx:hover, a.dmx.router-link-exact-active { background: var(--c-dmx); }

</style>
