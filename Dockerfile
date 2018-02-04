FROM fedora 
COPY dockerdata/usr/local/bbsd /usr/local/bbsd 
COPY dockerdata//usr/bin/tdftool /usr/bin/tdftool 
COPY dockerdata/etc/passwd /etc/passwd
COPY dockerdata/etc/group /etc/group

WORKDIR /usr/local/bbsd 
CMD ["/usr/local/bbsd/pmain"]
