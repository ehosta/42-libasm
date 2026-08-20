FROM --platform=linux/amd64 alpine:3.20

RUN apk add --no-cache \
    make \
    gcc \
    musl-dev \
    nasm \
    valgrind \
    clang \
    tini

ENV TERM=xterm

WORKDIR /app
COPY . /app

ENTRYPOINT ["/sbin/tini", "--"]
