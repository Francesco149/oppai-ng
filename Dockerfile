ARG PREFIX=
FROM ${PREFIX}ubuntu:bionic
RUN apt-get update && apt-get install -y \
  gcc musl musl-tools musl-dev git-core file
WORKDIR /tmp
CMD setarch $arch ./release
