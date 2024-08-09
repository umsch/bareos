#!/usr/bin/env bash

podman build \
        -f Dockerfile \
        -t bareos:fedora40-grpc

podman run \
        --network bridge \
        --name bareos \
        --hostname bareos \
        --privileged  \
        --replace \
        -v .:/source:Z \
        -v ./build:/build:Z \
        -p 13343:13343 \
        -t -i \
        bareos:fedora40-grpc \
        /bin/bash -c '/source/grpc_build.sh'