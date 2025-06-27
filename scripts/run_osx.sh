#!/bin/bash

make fclean
docker container stop libasm-ubuntu
docker container rm libasm-ubuntu
docker build --platform linux/amd64 -t pvndv/libasm-ubuntu -f ./scripts/Dockerfile .
docker run --platform linux/amd64 --name libasm-ubuntu pvndv/libasm-ubuntu
