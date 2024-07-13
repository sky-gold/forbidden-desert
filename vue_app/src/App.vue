<template>
  <div v-if="isLogin">
    <div v-if="('first_name' in this.cookies.get('tg_user') && ('last_name' in this.cookies.get('tg_user')))">
      {{this.cookies.get('tg_user').first_name}} {{this.cookies.get('tg_user').last_name}}
    </div>
    <div v-else-if="('first_name' in this.cookies.get('tg_user'))">
      {{this.cookies.get('tg_user').first_name}}
    </div>
    <div v-else>
      {{ this.cookies.get('tg_user').username }}
    </div>
    <button @click="logout" id="logout">Выход</button>
  </div>
  <div v-else>
    <div ref="telegram"></div>
  </div>
 </template>
 
 <script>
 import { useCookies } from "vue3-cookies";
 import { ref } from 'vue'
 
 export default {
   setup() {
     const { cookies } = useCookies();
     const isLogin = ref(cookies.isKey("tg_user"));
     return { cookies, isLogin};
   },
 
   // <data, methods...>
   
   async mounted() {
     if (!this.isLogin) {
       const script = document.createElement('script')
       script.async = true
       script.src = 'https://telegram.org/js/telegram-widget.js?2'
 
       
       script.setAttribute('data-size', "large")
       script.setAttribute('data-userpic', false)
       script.setAttribute('data-telegram-login', "suka_rabotat_budesh_bot")
       script.setAttribute('data-request-access', "write")
 
       window.onTelegramAuth = this.onTelegramAuth
       script.setAttribute('data-onauth', 'window.onTelegramAuth(user)')
 
       this.$refs.telegram.appendChild(script)
     }
   },

   updated() {
    if (!this.isLogin) {
       const script = document.createElement('script')
       script.async = true
       script.src = 'https://telegram.org/js/telegram-widget.js?2'
 
       
       script.setAttribute('data-size', "large")
       script.setAttribute('data-userpic', false)
       script.setAttribute('data-telegram-login', "suka_rabotat_budesh_bot")
       script.setAttribute('data-request-access', "write")
 
       window.onTelegramAuth = this.onTelegramAuth
       script.setAttribute('data-onauth', 'window.onTelegramAuth(user)')
 
       this.$refs.telegram.appendChild(script)
     }
   },
   
   methods: {
    async onTelegramAuth(user) {
      let res = await fetch('http://185.255.132.48:8000/auth', {
        method : 'GET',
        headers : { 
          'Authorization': unescape(encodeURIComponent(JSON.stringify(user))),
        }
      });
      if (res.ok) {
       this.cookies.set("tg_user", user, null, null, null, false, "Lax");
       this.isLogin = true;
       alert('Logged in as ' + user.first_name + ' ' + user.last_name + ' (' + user.id + (user.username ? ', @' + user.username : '') + ')');
      } else {
        alert("Не удалась авторизация");
      }
     }, 
     logout() {
       this.cookies.remove("tg_user");
       this.isLogin = false;
     }
   }
 }
 </script>