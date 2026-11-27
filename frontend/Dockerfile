FROM nginxinc/nginx-unprivileged:1.30.6-alpine

COPY frontend/nginx.conf /etc/nginx/conf.d/default.conf
COPY frontend/index.html frontend/app.js frontend/style.css /usr/share/nginx/html/
COPY maps/*.csv /usr/share/nginx/html/maps/

