FROM jjaguayo/ubuntu-pystorm:latest
COPY run_build_and_test.sh /run_build_and_test.sh
COPY pystorm/ /home/git/pystorm/
RUN chmod +x /run_build_and_test.sh
CMD ["/bin/bash","-lc","/run_build_and_test.sh"]
