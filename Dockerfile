 FROM python:alpine AS base 

RUN mkdir /app

ADD . /app

RUN apk add libc6-compat

CMD [ "/app/txf_file_server3" ]