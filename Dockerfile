FROM archlinux:latest

RUN pacman --needed --noconfirm --noprogressbar -Sy \
     rsync \
     python-tox \
     gcc \
     git \
     make

ADD tox.ini      /code/
ADD Makefile     /code/
ADD src/*.c      /code/src/
ADD test/*.py    /code/test/
ADD util/*.h     /code/util/
ADD vendor/*.h   /code/vendor/
ADD vendor/*.c   /code/vendor/
ADD scripts      /code/scripts
ADD .git         /code/.git
ADD readme.md    /code/readme.md

RUN cd /code && tox --notest

RUN cd /code && \
    make -j && \
    mv -fv bin/* /usr/local/bin
