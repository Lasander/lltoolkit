FROM ubuntu:14.04

# Avoid ERROR: invoke-rc.d: policy-rc.d denied execution of start.
RUN sed -i "s/^exit 101$/exit 0/" /usr/sbin/policy-rc.d

# Install basic
RUN \
  export DEBIAN_FRONTEND=noninteractive && \
  sed -i 's/# \(.*multiverse$\)/\1/g' /etc/apt/sources.list && \
  apt-get update -y && \
  apt-get -y upgrade && \
  apt-get install -y build-essential && \
  apt-get install -y software-properties-common && \
  apt-get install -y byobu curl git htop man unzip vim wget

# Install clang-4, clang-5, libc++, gcc-7, cmake
RUN \
  export DEBIAN_FRONTEND=noninteractive && \
  echo "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty main" >> /etc/apt/sources.list && \
  echo "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-4.0 main" >> /etc/apt/sources.list && \
  echo "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-5.0 main" >> /etc/apt/sources.list && \
  wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add - && \
  sudo apt-key adv --keyserver ha.pool.sks-keyservers.net --recv-keys 1E9377A2BA9EF27F && \
  apt-add-repository -y ppa:ubuntu-toolchain-r/test && \
  add-apt-repository -y ppa:george-edison55/cmake-3.x && \
  apt-get update -y && \
  apt-get -yq --no-install-suggests --no-install-recommends --force-yes install clang-5.0 clang-4.0 libc++-dev libc++abi-dev && \
  apt-get -yq --no-install-suggests --no-install-recommends --force-yes install g++-7 g++-6 && \
  apt-get -yq --no-install-suggests --no-install-recommends --force-yes install cmake && \
  rm -rf /var/lib/apt/lists/*

# Set environment variables.
ENV HOME /root

# Define working directory.
WORKDIR /root

# Define default command.
CMD ["bash"]
