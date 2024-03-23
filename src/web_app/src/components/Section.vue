<template>
	<div v-if="visible" class="section" :class="[cat, { collapsable: collapsable }]">
		<div v-if="title" class="main">
			<h3 @click="toggleVisibility">{{title}}<span v-show="collapsed"> [+]</span></h3>
			<div v-if="!collapsed" class="content">
				<slot/>
			</div>
		</div>
		<div v-else-if="!title && subtitle" class="sub">
			<h5 v-html="subtitle"></h5>
			<slot/>
		</div>
	</div>
</template>

<script>
export default {
  name: 'Section',
  props: {
  	collapsable: Boolean,
  	cat: String,
  	title: String,
  	subtitle: String,
  	visibleCats: {
  		type: Array,
  		default: () => []
  	}
  },
  data() {
  	return {
  		collapsed: false,
  		collapsedPrefKey: 'Section_' + this.title + '_c'
  	}
  },
  computed: {
  	visible() {
  		if (this.visibleCats.length > 0)
  			return this.visibleCats.indexOf(this.cat) > -1
  		else
  			return true
  	}
  },
  methods: {
  	toggleVisibility() {
  		if (this.collapsable) {
  			this.collapsed = !this.collapsed
  			if (this.collapsed)
				this.$root.setPref(this.collapsedPrefKey, this.collapsed)
			else
				this.$root.delPref(this.collapsedPrefKey)
  		}
  	}
  },
  mounted() {
  	let _collapsed = this.$root.getPref(this.collapsedPrefKey)
  	if (_collapsed != null)
		this.collapsed = _collapsed
  }
}
</script>

<style scoped>
.section {
	display: flex;
	flex-direction: column;
	/*user-select: none;*/
}
.main {
	margin: 10px 0 0;
}
.section:first-of-type .main {
	margin: 0;
}
.sub {
	margin: 2px 0 0;
}
.sub > h5 {
	background: rgb(238, 241, 242);
	margin: 0;
	padding: 4px;
	text-align: center;
}
.content {
	margin-top: 4px;
}
.content > div:not(:last-child):not(.section) {
	margin-bottom: 8px;
}
.section.centered .content { text-align: center; }
.section.info .content {
	background: var(--c-content-bg);
	padding: 10px 8px;
}
.section.info .sub p {
	padding: 0 4px;
}
h3 {
	font: 1.1em Arial;
	color: white;
	background: var(--c-neutral);
	margin: 0;
	padding: .3em;
	text-align: center;
	text-transform: uppercase;
}
.section.collapsable h3:hover {
	cursor: pointer;
}
.error h3 { background: var(--c-error); }
.misc h3 { background: var(--c-misc); }
.dmx h3 { background: var(--c-dmx); }

/deep/ p {
	display: table;
	table-layout: fixed;
	box-sizing: border-box;
	width: 100%;
	margin: 2px 0;
	padding: 2px 0 0;
	line-height: 1.5em;
}
/deep/ p > b {
	font-weight: normal;
	background: rgb(238, 241, 242);
	display: table-cell;
	width: 46%;
	padding-right: 20px;
	text-align: right;
}
</style>
