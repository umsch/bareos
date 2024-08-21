# Basis-Stufe mit den installierten Paketen
FROM bareos:fedora40 as bareos_system

LABEL description="build and test bareos grpc environment"

RUN dnf install -y grpc-devel procps hostname grpcurl jq python3-selenium php-cli protobuf-compiler protobuf-devel

#FROM bareos_system AS build

#ADD . /source

#WORKDIR /bareos
#RUN cmake -DUSE_SYSTEM_FMT=ON -S /source -B . -G Ninja
#RUN cmake --build .

#FROM build AS bareos

#WORKDIR /bareos
#COPY --from=build /bareos .
