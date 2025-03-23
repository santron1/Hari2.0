import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vitejs.dev/config/
export default defineConfig({
  server: {
    proxy: {
      '/api': {
        target: 'http://192.168.43.241', // Your backend server address
        changeOrigin: true,
        rewrite: (path) => path.replace(/^\/api/, '') // Removes '/api' from the request URL
      }
    }
  },
  plugins: [react()],
})

