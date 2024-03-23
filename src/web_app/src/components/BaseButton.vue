<template>
	<button :class="{ [color]: true, hasSpinner: spinner }" @click="$emit('onclicked')">
		<span v-if="spinner" class="spinner-container">
			<i class="spinner"></i>
		</span>
		<span class="text">
			{{text}}
		</span>
	</button>
</template>

<script>
export default {
  name: 'BaseButton',
  props: {
  	text: String,
  	color: String,
	spinner: Boolean
  }
}
</script>

<style scoped>
button {
	font: .8em Arial;
	font-kerning: none;
	color: #fff;
	background: var(--c-neutral);
	position: relative;
	margin: 6px 0 0;
	padding: 6px 20px;
	border: 0;
	/*border-radius: 6px;*/
	box-shadow: 0 6px rgb(180, 180, 180);
	text-transform: uppercase;
	/*transition: all .1s ease-out;*/
	cursor: pointer;
	transform: translateY(-6px);
}
button:hover {
	top: 2px;
	box-shadow: 0 4px rgb(180, 180, 180);
}
button:active {
	top: 6px;
	box-shadow: none !important;
	outline: 0;
}
button:focus {
	outline: 0;
}

button.yellow {
	background: rgb(255, 220, 118);
	box-shadow: 0 6px rgb(248, 184, 74);
}
button.yellow:hover {
	box-shadow: 0 4px rgb(248, 184, 74);
}

button.red {
	background: rgb(255, 171, 171);
	box-shadow: 0 6px rgb(240, 128, 128);
}
button.red:hover {
	box-shadow: 0 4px rgb(240, 128, 128);
}

button * {
	pointer-events: none;
}
button.hasSpinner > .text {
	visibility: hidden;
}

.spinner-container {
	position: absolute;
	top: calc(50% - 10px);
	left: calc(50% - 10px);
}
.spinner {
	display: inline-block;
	height: 20px;
	width: 20px;
	animation: spin 5.4s infinite;
}
.spinner::after {
	content: '';
	display: inline-block;
	width: 100%;
	height: 100%;
	border-width: 3px;
	border-color: white white transparent transparent;
	border-style: solid;
	border-radius: 20px;
	box-sizing: border-box;
	animation: spin 0.7s ease-in-out infinite;
}
@keyframes spin {
	0% { transform: rotate(0deg) }
	100% { transform: rotate(360deg) }
}
</style>
