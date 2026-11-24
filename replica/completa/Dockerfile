# Mismo entorno que el Dockerfile del artefacto oficial (Ubuntu 22.04, g++-12, jemalloc, numactl).
# Únicos cambios: se añaden time (pico de memoria) y patch, y se quita matplotlib (las gráficas se hacen fuera).
FROM ubuntu:22.04
RUN echo 'APT::Install-Suggests "0";' >> /etc/apt/apt.conf.d/00-docker
RUN echo 'APT::Install-Recommends "0";' >> /etc/apt/apt.conf.d/00-docker
RUN DEBIAN_FRONTEND=noninteractive \
  apt-get update \
  && apt install -y python3 git cmake ca-certificates g++-12 build-essential \
  libssl-dev libboost-all-dev ninja-build libsodium-dev libdouble-conversion-dev \
  liblz4-dev libdwarf-dev liblzma-dev libiberty-dev libgflags-dev libzstd-dev \
  libsnappy-dev libunwind-dev libaio-dev libgoogle-glog-dev libgmock-dev python3-pip \
  numactl libjemalloc-dev time patch \
  && rm -rf /var/lib/apt/lists/*
RUN useradd -ms /bin/bash apprunner
USER apprunner
