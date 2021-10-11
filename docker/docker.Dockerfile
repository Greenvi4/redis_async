#Download base ubuntu image
FROM ubuntu:20.04

LABEL version="1.0" description="Image for building redis_async"
ARG DEBIAN_FRONTEND=noninteractive

#ENV PATH ""
#ENV LD_LIBRARY_PATH ""

RUN apt-get update && apt-get install -y build-essential vim libboost-all-dev redis-server liblog4cxx-dev cmake

