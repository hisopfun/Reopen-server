FROM alpine as build-env
RUN apk add libc6-compat
RUN apk add --no-cache build-base
WORKDIR /app
COPY . .
# Compile the binaries
RUN gcc -o txf_file_server3 txf_file_server3.c


FROM alpine
RUN apk add libc6-compat
RUN apk add --no-cache build-base
COPY --from=build-env /app /app
WORKDIR /app
CMD ["/app/txf_file_server3"] 