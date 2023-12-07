import Vue from 'vue'
import Router from 'vue-router'

// Import components
import Config from '@/components/Config'
import Status from '@/components/Status'
import Dmx from '@/components/Dmx'

Vue.use(Router)

export default new Router({
  routes: [
    {
    	path: '/',
    	name: 'Configuration',
    	component: Config
    },
    {
        path: '/status',
        name: 'Status',
        component: Status
    },
    {
    	path: '/output',
    	name: 'Output',
    	component: Dmx
    }
  ],
  mode: 'hash'
})
