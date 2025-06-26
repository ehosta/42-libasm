FROM debian:bookworm

ENV TERM=xterm-256color

COPY . libasm/

RUN apt update && apt install -y \
    build-essential \
    nasm \
    make \
    && apt clean

WORKDIR /app
CMD ["/bin/bash"]