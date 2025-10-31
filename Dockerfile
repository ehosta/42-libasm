FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y make gcc nasm valgrind clang && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENV TERM=xterm

COPY . /app
WORKDIR /app

RUN make -C tests/
