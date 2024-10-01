FROM python:3.12
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
#ENV HOME=${home}
# We may want to pass in some configuration via env?
#COPY Pipfile Pipfile.lock ./
WORKDIR /tmp
COPY Pipfile /tmp/
RUN pip install --upgrade pipenv 'pip>=24.0' && \
  pipenv install --skip-lock --deploy --system && \
  rm -frv .cache/pip
RUN mkdir /data && pip install bacpypes3
WORKDIR /data
##BIND MOUNT##COPY *.py *.ini /data/
ENTRYPOINT ["/data/main.py"]
CMD []
#VOLUME ["/data"]
