#syntax=docker/dockerfile:1

FROM node:lts

# Install a particular NPM version because newer vsersions tend to hang during audit checks
ENV NPM_VERSION=10.3.0
RUN npm install -g npm@"${NPM_VERSION}"
# This can probably be commented out at some point in the future when NPM is fixed.

#
# NOTE: docker-compose's bind mount of the app only happens AFTER the
# build phase!
#

#RUN test -d /app
RUN test -d /home/node
#RUN test -d /home/node/app

USER root

#RUN mkdir -p /home/node/app/node_modules && test -d /app && test -d /app/node_modules
RUN umask 0002 && mkdir -p /home/node/app/node_modules/.cache  /home/node/node_modules && chown -R node:node /home/node
WORKDIR /home/node/

ENV PATH /home/node/node_modules/.bin:$PATH
ENV REACT_APP_DOCKERENV=true

USER node

COPY package.json ./
RUN npm install && npm cache clean --force

WORKDIR /home/node/app

#ADD src ./src/
#ADD public ./public/

EXPOSE 3000
EXPOSE 35729

#ENTRYPOINT ["/bin/bash", "/app/run.sh"]
#CMD ["start"]

CMD ["npm", "start"]
