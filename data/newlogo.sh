#!/bin/sh
export GOPATH=/home/dan/git-remote

~/.npm-global/bin/cfonts \
    --align center -f block "IGC|SOFTWARE" -c blue,yellow   | \
    ./cp437 2>/dev/null | \
    go run /home/dan/git-remote/src/ansiart2utf8/ansiart2utf8.go -w 78 - | \
    ./cp437 2>/dev/null > igclogo6.ans 

