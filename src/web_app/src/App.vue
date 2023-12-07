<template>
  <div id="app">
    <input type="checkbox" id="theme-switch" v-model="darkTheme">
    <h2 id="title">{{config.name}}</h2>
    <h5 id="subtitle">@ {{config.ip}}</h5>
    <Nav/>
    <div id="content">
      <keep-alive> <!-- keeps components loaded even when not showed -->
        <router-view/>
      </keep-alive>
    </div>
    <Footer/>
  </div>
</template>

<script>
  import Nav from '@/components/Nav'
  import Footer from '@/components/Footer'

  export default {
    name: 'App',
    components: {
      Nav,
      Footer
    },
    data() {
      return {
        darkTheme: 0
      }
    },
    computed: {
      config() {
        return this.$root.$data.config
      }
    },
    watch: {
      darkTheme(v) {
        let d = document.documentElement;
        let a = "dark";
        if (v) d.setAttribute(a, v)
        else d.removeAttribute(a);
        this.$root.setPref(a, this.darkTheme)
      }
    },
    mounted() {
      this.darkTheme = this.$root.getPref("dark")
    }
  }
</script>

<style>
:root {
  --c-bg: #fff;
  --c-txt: rgb(72, 72, 72);

  --c-neutral: rgb(210, 210, 210);

  --c-misc: #c9dce4;
  --c-dmx: #ded4ea;
  --c-mqtt: rgb(255, 216, 187);
  --c-error: rgb(255, 80, 100);

  --c-content-bg: rgb(250, 250, 250);
}
[dark] {
  filter: invert(1) brightness(2.5) hue-rotate(180deg);
}

body {
  font: 1em Arial;
  color: var(--c-txt);
  background: var(--c-bg);
  margin: 15px 0 10px;
}
a {
  color: inherit;
  cursor: pointer;
  text-decoration: none;
}
p { font-family: Tahoma; margin: 0; padding: 0; }
h2, h3 { font-weight: 100; }
h3 { margin: 20px 0 15px; }

* {
  cursor: default;
}

p > i {
  font-size: 14px;
  margin: 10px;
}

::-moz-focus-inner { border: 0; }

.v-spinner {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
  background: rgb(0, 0, 0, .7);
  pointer-events: none;
  z-index: 100;
}

#app {
  /*background: white;*/
  width: 95%;
  /*height: 80%;*/
  margin: 0 auto;
  /*padding: 10px 30px 30px;*/
}
#content {
  padding: 20px 2%;
  /*border: 6px solid rgb(201, 220, 228);
  border-style: solid none;*/
}
#content > div {
  display: flex;
  flex-direction: column;
}

::selection { color: white; background: rgb(189, 189, 189); }
::-moz-selection { color: white; background: rgb(189, 189, 189); }

#theme-switch {
  font-size: 1.6em;
  position: absolute;
  top: 4%;
  right: 5%;
  appearance: none;
  cursor: pointer;
}
#theme-switch::before {
  content: '🌙';
}
#theme-switch:checked::before {
  content: '☀️';
}

#title {
  color: rgb(100, 100, 100);
  font: bold 1.5em Arial;
  letter-spacing: -1px;
  /*background: rgb(255, 242, 225);
  padding: 2px 20px;*/
  margin: 30px 0 0;
  text-align: center;
  /*box-shadow: 1px 1px 3px rgba(0, 0, 0, .5);*/
}
#subtitle {
  color: rgb(200, 200, 200);
  font: 1.2em Arial;
  margin: 4px 0 18px;
  text-align: center;
}

a span.label {
  cursor: pointer;
}
a span.label::after {
  content: ' ';
  display: inline-block;
  background: #acacac;
  margin-left: 6px;
  padding: 6px;
  border-radius: 100%;
}
#nav a span.label::after { margin-right: -6px; }
a.on span.label::after {
  background: #8effb0;
}
a.on.anim span.label::after{
  animation: glow .5s step-end infinite;
}
@keyframes glow {
  0%   { background-color: #9de8b4;; }
  50% { background-color: #8effb0; }
}
</style>
