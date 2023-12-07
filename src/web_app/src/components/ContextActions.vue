<template>
	<div ref="self" class="context-actions">
		<slot/>
	</div>
</template>

<script>
export default {
  name: 'ContextActions',
  mounted() {
  	let navEl = this.$root.$el.querySelector('#nav')
  	navEl.appendChild(this.$refs.self)
  },
  beforeDestroy() {
  	this.$refs.self.remove()
  }
}
</script>

<style scoped>
.context-actions {
  /*position: relative;
  top: -15px;
  float: right;*/
  /*align-self: end;*/

  /*position: fixed;
  top: 1em;
  right: 0;
  width: 100%;
  padding: 0 4.4%;
  text-align: right;
  z-index: 100;*/

  float: right;
  padding-right: 2%;
}
.context-actions button { margin: 0; }
.context-actions.btm {
  top: 0;
  margin: 25px 0 5px;
  float: none;
  text-align: right;
}
</style>
