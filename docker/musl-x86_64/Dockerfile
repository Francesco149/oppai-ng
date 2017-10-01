FROM multiarch/alpine:x86_64-latest-stable

RUN apk add --no-cache musl-dev gcc git xz
WORKDIR /tmp
CMD [ "sh", "./release" ]
