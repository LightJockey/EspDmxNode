<template>
	<p :class="{ editing: editing }">
		<b v-html="title"></b>
		<span v-if="isInput" class="input-container">
			<input
				:disabled="readonly || !editing"
				:type="password ? editing ? 'text' : 'password' : false"
				:value="value"
				@input="update"
				autocomplete="off">
			<slot name="head"/>
		</span>
		<label v-if="isDropdown">
			<select :disabled="readonly || !editing"
					@change="update">
				<option v-for="(option, i) in options" :value="i" :selected="value == i">{{option}}</option>
			</select>
		</label>
		<slot/>
	</p>
</template>

<script>
export default {
  name: 'ConfigField',
  props: {
  	type: String,
  	readonly: Boolean,
  	password: Boolean,
  	options: Array,
  	title: String,
  	description: String,
  	editing: Boolean,
  	value: {}
  },
  computed: {
	isInput() { return this.type == 'input' },
	isDropdown() { return this.type == 'dropdown' }
  },
  methods: {
  	update(e) {
  		this.$emit('input', e.target.value)
  	}
  }
}
</script>

<style scoped>
.input-container {
	position: relative;
	display: flex;
	gap: 4px;
}

input, select {
	font: .8em Verdana;
	color: rgb(100, 100, 100);
	background: transparent/*rgb(118, 179, 203)*/;
	width: 100%;
	padding: 0 12px;
	line-height: 24px;
	box-sizing: padding-box;
	border: 0;
	border-bottom: 4px solid rgb(238, 241, 242);
	appearance: none;
	-moz-appearance: none;
	outline: none;
	box-sizing: border-box;
}
.editing input:not(:disabled):hover {
	cursor: auto;
}
.editing select:not(:disabled):hover {
	cursor: pointer;
}
input:focus, select:focus { outline: none; }
label { position: relative; }
label::after {
	content: '>';
	font: bold 14px "Consolas", monospace;
	color: rgb(238, 241, 242);
	position: absolute;
	top: 3px;
	right: 8px;
	padding: 0 0 2px;
	pointer-events: none;
	transform: rotate(90deg);
}
.editing label::after {
	color: rgb(248, 184, 74);
}

input[type="file"] {
	margin-bottom: 10px;
	padding: 0;
	border: 0;
}

.editing :disabled {
	color: rgb(200, 200, 200);
}
.editing input:not(:disabled), .editing select:not(:disabled) {
	color: rgb(130, 130, 130);
	background: rgb(255, 253, 230);
	border-bottom-color: rgb(248, 184, 74);
}
.editing input:not(:disabled):hover, .editing input:not(:disabled):focus, .editing select:not(:disabled):hover, .editing select:not(:disabled):focus {
	color: rgb(110, 110, 110);
	background: rgb(255, 231, 166);
}
.editing input:not(:disabled)::selection, .editing select:not(:disabled)::selection {
	color: white;
	background: rgb(248, 184, 74);
}
.editing input:not(:disabled)::-moz-selection, .editing select:not(:disabled)::-moz-selection {
	color: white;
	background: rgb(248, 184, 74);
}

.editing.error input, .editing.error select {
	background: rgb(248, 128, 128) !important;
	border-bottom-color: rgb(248, 40, 12) !important;
}
/*.editing.dirty input, .editing.dirty select {
	background: rgb(255, 200, 124);
	border-bottom-color: rgb(248, 120, 32);
}*/
</style>
