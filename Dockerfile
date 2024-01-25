FROM ubuntu:22.04

# Install all necessary dependencies
RUN \
  apt-get update && \
  apt-get -y upgrade && \
  apt-get install -y build-essential && \
  apt-get install -y software-properties-common && \
  apt-get install -y git htop man unzip vim wget nano && \
  apt-get install -y cmake lbzip2 python3 && \
  apt-get -y clean && \
  apt-get -y autoclean && \
  apt-get -y autoremove

# Set environment variables.
ENV HOME /root
ENV EMSDK /root/emsdk
ENV EMSCRIPTEN_VERSION 3.1.51
ENV INNOEXTRACT /root/innoextract

# Define working directory.
WORKDIR /root

# Install Emscripten
RUN \
  git clone https://github.com/emscripten-core/emsdk.git && \
  cd ${EMSDK} && \
  ./emsdk install ${EMSCRIPTEN_VERSION} && \
  echo "## Done"

RUN  \
  cd ${EMSDK} && \
  echo "## Generate standard configuration" && \
  ./emsdk activate ${EMSCRIPTEN_VERSION} && \
  chmod 777 ${EMSDK}/upstream/emscripten && \
  chmod -R 777 ${EMSDK}/upstream/emscripten/cache && \
  echo "int main() { return 0; }" > hello.c && \
  ${EMSDK}/upstream/emscripten/emcc -c hello.c && \
  cat ${EMSDK}/upstream/emscripten/cache/sanity.txt && \
  echo "## Done"

# Cleanup Emscripten installation and strip some symbols
RUN \
  echo "## Aggressive optimization: Remove debug symbols" && \
  cd ${EMSDK} && . ./emsdk_env.sh && \
  # Remove debugging symbols from embedded node (extra 7MB&& )
  strip -s `which node` && \
  # Tests consume ~80MB disc spac&& e
  rm -fr ${EMSDK}/upstream/emscripten/tests && \
  # Fastcomp is not supporte&& d
  rm -fr ${EMSDK}/upstream/fastcomp && \
  # strip out symbols from clang (~extra 50MB disc space&& )
  find ${EMSDK}/upstream/bin -type f -exec strip -s {} + || true && \
  echo "## Done"

# Add Emscripten to PATH and set umask
ENV EMSDK=${EMSDK} \
  EMSDK_NODE=${EMSDK}/node/15.14.0_64bit/bin/node \
  PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${EMSDK}/upstream/bin:${EMSDK}/node/15.14.0_64bit/bin:${PATH}"

RUN \
  echo "umask 0000" >> /etc/bash.bashrc && \
  echo ". ${EMSDK}/emsdk_env.sh" >> /etc/bash.bashrc && \
  echo "## Done" && \
  echo "## Emscripten installed successfully!"

# Install innoextract
ADD . ${INNOEXTRACT}
RUN \
  cd innoextract && \
  ./build.sh

RUN \
  echo "Build done."

WORKDIR ${INNOEXTRACT}

CMD ["python3", "-m", "http.server", "8000"]