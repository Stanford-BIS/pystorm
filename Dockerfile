FROM jjaguayo/ubuntu-pystorm:latest

COPY docker/run_build.sh /run_build.sh
COPY docker/run_test.sh /run_test.sh
COPY ./ /home/git/pystorm/

RUN chmod +x /run_build.sh \
    && /run_build.sh

CMD ["/bin/bash"]
