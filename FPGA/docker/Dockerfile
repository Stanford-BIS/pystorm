FROM centos:6.8

RUN yum update -y && yum install -y xterm sudo yum-utils
RUN yum-config-manager --add-repo http://fishshell.com/files/linux/RedHat_RHEL-6/fish.release:2.repo
RUN yum install -y fish
RUN yum install -y dejavu-sans-fonts.noarch dejavu-sans-mono-fonts.noarch dejavu-serif-fonts.noarch
RUN yum install -y libpng vim
RUN yum install -y libX11.i686 libXext.i686 libXft.i686 
