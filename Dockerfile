FROM archlinux:latest

RUN pacman --needed --noconfirm --noprogressbar -Sy \
     python \
     gcc \
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

RUN cd /code && \
    make -j && \
    mv -fv bin/* /bin

CMD /bin/_gen_csv -h; \
    /bin/_gen_bsv -h; \
    cd /code/src; \
    for name in $(ls | grep -v ^_); do \
      /bin/$(echo $name | cut -d. -f1) -h; \
    done; \
    true
