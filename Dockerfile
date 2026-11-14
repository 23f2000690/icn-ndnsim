# Official ndnSIM install, packaged so it runs on Windows.
# Upstream procedure: https://ndnsim.net/current/getting-started.html
# Ubuntu 20.04 matches the platforms ndnSIM documents (ns-3.35 + waf).

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive \
    TZ=Etc/UTC \
    PYTHONDONTWRITEBYTECODE=1

# Boost and a few other build packages live in Ubuntu's universe archive.
RUN printf '%s\n' \
        'deb http://archive.ubuntu.com/ubuntu focal universe' \
        'deb http://archive.ubuntu.com/ubuntu focal-updates universe' \
        'deb http://security.ubuntu.com/ubuntu focal-security universe' \
        >> /etc/apt/sources.list

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        git \
        libboost-all-dev \
        libsqlite3-dev \
        libssl-dev \
        pkg-config \
        python3 \
    && ln -sf /usr/bin/python3 /usr/bin/python \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/ndnSIM

# ns-3 fork that ndnSIM expects, then ndnSIM itself (ndn-cxx and NFD come in as submodules).
RUN git clone https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3 \
    && git clone --recursive https://github.com/named-data-ndnSIM/ndnSIM.git ns-3/src/ndnSIM

WORKDIR /opt/ndnSIM/ns-3

# Python bindings and the visualizer are omitted. They are optional and
# do not help a headless run from Windows. Scenarios still compile and run.
RUN ./waf configure --enable-examples --disable-python \
    && ./waf -j2

WORKDIR /opt/ndnSIM/ns-3
CMD ["./waf", "--run", "wired-topology --results=/results"]
