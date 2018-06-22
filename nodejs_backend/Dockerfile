FROM node:carbon

RUN mkdir /service
WORKDIR /service

COPY package*.json ./

RUN npm i -g
RUN npm i sleep
RUN mv ./node_modules/* /usr/local/lib/node_modules/
RUN rm -rf ./node_modules

ENV NODE_PATH=/usr/local/lib/node_modules
