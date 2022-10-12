FROM ubuntu:latest
LABEL description="Container for use with Visual Studio" 

RUN apt-get update && apt-get install -y g++ rsync zip openssh-server cmake make nano

RUN mkdir -p /var/run/sshd

RUN echo 'PasswordAuthentication yes' >> /etc/ssh/sshd_config && \ 
   ssh-keygen -A 

EXPOSE 22


COPY . /usr/src/LAB1
WORKDIR /usr/src/LAB1