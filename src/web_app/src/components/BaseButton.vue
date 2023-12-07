<template>
	<button :class="color" @click="$emit('onclicked')">{{text}}</button>
</template>

<script>
export default {
  name: 'BaseButton',
  props: {
  	text: String,
  	color: String
  }
}
</script>

<style scoped>
button {
	font: .8em Arial;
	font-kerning: none;
	color: #fff;
	/*background: rgb(189, 189, 189);*/
	background: var(--c-neutral);
	position: relative;
	display: inline-block;
	/*margin: 30px 0 12px;*/
	/*margin-bottom: 12px;*/
	margin: 6px 0;
	padding: 6px 20px;
	border: 0;
	/*border-radius: 6px;*/
	/*box-shadow: 0 6px rgb(156, 156, 156);*/
	box-shadow: 0 6px rgb(180, 180, 180);
	text-shadow: 0 1px 6px rgba(100, 100, 100, .8);
	text-transform: uppercase;
	/*transition: all .1s ease-out;*/
	cursor: pointer;
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
	background: rgb(255, 225, 139);
	box-shadow: 0 6px rgb(248, 184, 74);
	text-shadow: 0 1px 6px rgba(153, 68, 13, .8);
}
button.yellow:hover {
	box-shadow: 0 4px rgb(248, 184, 74);
}
button.red {
	background: rgb(255, 171, 171);
	box-shadow: 0 6px rgb(240, 128, 128);
	text-shadow: 0 1px 6px rgba(125, 33, 33, .8);
}
button.red:hover {
	box-shadow: 0 4px rgb(240, 128, 128);
}
</style>
