 FROM python:alpine AS base 

RUN mkdir /app

ADD . /app

RUN apk add libc6-compat

RUN apk add build-base

RUN gcc /app/txf_file_server3.c -o /app/txf_file_server3

CMD [ "/app/txf_file_server3" ]