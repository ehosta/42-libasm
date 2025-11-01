FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y make gcc nasm valgrind clang tini && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENV TERM=xterm

COPY . /app
WORKDIR /app
