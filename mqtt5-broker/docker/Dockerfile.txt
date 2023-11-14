FROM ubuntu:20.04

COPY ../config ./mqtt5/config
COPY ../docker-compose.yml ./mqtt5

RUN apt-get install docker-compose

EXPOSE 1883
EXPOSE 9001