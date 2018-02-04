FROM fedora 
COPY . /usr/local/bbsd 
COPY . /usr/bin/tdftool 
COPY dockerdata/etc/passwd /etc/passwd
COPY dockerdata/etc/group /etc/group

WORKDIR /usr/local/bbsd 
CMD ["/usr/local/bbsd/pmain"]
