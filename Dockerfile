FROM ubuntu:latest

RUN apt-get update && apt-get install -y g++ cmake make

COPY . /usr/src/LAB1
WORKDIR /usr/src/LAB1

RUN cmake -S . -B ./build
RUN cmake --build ./build

CMD ./build/main