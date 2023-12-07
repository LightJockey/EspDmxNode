<template>
	<span class="dmx-channel">
		<b>{{index + 1}}</b>
		<i :style="{ background: valueBackground }">{{f_value}}</i>
	</span>
</template>

<script>
export default {
  name: 'DmxChannel',
  props: {
  	index: Number,
  	value: Number,
  	viewType: Number
  },
  computed: {
  	f_value() {
  		let v = ''
  		switch (this.viewType) {
  			case 0: // Percentage
  				let vp = this.valuePercentage
  				if (vp && vp > 0) {
  					if (vp < 10)
  						v += '0' + vp
  					else
  						v += vp > 99 ? 'FL' : vp
  				}
  				else
  					v += '00'
  				break
			case 1: // Decimal
				v += this.value ? this.value : 0
				break
			case 2: // Hex
				let vh = this.value ? this.value.toString(16).toUpperCase() : '00'
				v += vh.length == 1 ? vh + vh : vh
				break
  		}
  		return v
  	},
  	valuePercentage() {
  		return Math.round(this.value * 100 / 255)
  	},
  	valueBackground() {
  		let vp = this.valuePercentage
  		return !isNaN(vp) ? 'rgba(201, 220, 228,' + (vp / 100) + ')' : 'rgb(60,60,60)'
  	}
  }
}
</script>

<style scoped>
.dmx-channel {
	display: inline-block;
	/*width: 16px;
	padding: 2px 6px;
	line-height: 18px;*/
	margin-right: 1px;
	text-align: center;
}
.dmx-channel b {
	font: normal 9px Arial;
	/*font-size: 9px;
	font-weight: normal;*/
	color: rgb(100, 100, 100);
	background: white;
	display: block;
	line-height: 12px;
}
.dmx-channel i {
	font: normal 10px Arial;
	/*font-size: 10px;
	font-style: normal;*/
	color: rgb(60, 60, 60);
	background: rgb(189, 189, 189);
	display: block;
	width: 20px;
	padding: 4px 0;
	line-height: 12px;
}
</style>
