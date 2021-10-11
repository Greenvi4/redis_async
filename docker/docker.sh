#!/bin/bash

SCRIPT_DIR=`dirname $(readlink -e "$0")`
WORKING_DIR=`readlink -e "$SCRIPT_DIR/.."`
install_dir="install"
build_dir="build_from_docker"
docker_image="greenvich/redis_async"
container_name="redis_async_dev"
version=$1

if [ -z "$version" ]; then
    version="latest"
fi

docker build -f $SCRIPT_DIR/docker.Dockerfile $SCRIPT_DIR/ -t $docker_image:$version

if [ $? -ne 0 ]; then
    echo -e "ERROR: Сборка docker-образа \"$docker_image:$version\" завершилась с ошибкой"
    exit 1
fi

if [[ ! `docker image ls | egrep "$docker_image[ ]+$version[ ]"` ]]; then
    echo -e "ERROR: Не найден docker-образ \"$docker_image:$version\""
    exit 1
fi

if [[ `docker ps -qa -f "name=$container_name"` ]]; then
    CONTAINER_OLD_ID=`docker ps -qa -f "name=$container_name"` 
    docker stop $CONTAINER_OLD_ID
    docker rm $CONTAINER_OLD_ID
fi

container_id=`docker run -it -d --name $container_name --volume $WORKING_DIR:/mnt $docker_image:$version /bin/bash`

docker exec $container_id bash -c \
    "cd /mnt ; \
    rm -rf $build_dir ;
    mkdir $build_dir && \
    mkdir -p $install_dir && \
    cd $build_dir && \
    cmake -DCMAKE_INSTALL_PREFIX=../$install_dir -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -- -j`nproc` && \
    cd .. ; \
    rm -rf $build_dir"

if [[ `docker ps -qa -f "name=$container_name"` ]]; then
    CONTAINER_ID=`docker ps -qa -f "name=$container_name"` 
    docker stop $CONTAINER_ID
    docker rm $CONTAINER_ID
fi

exit 0

