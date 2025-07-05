FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y make gcc nasm valgrind && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENV TERM=xterm

WORKDIR /app
COPY . .

RUN make fclean
RUN make

CMD ["make", "vg"]