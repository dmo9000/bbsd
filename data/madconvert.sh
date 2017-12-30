#!/bin/sh 

# I'm still not sure I believe this works at all. 

make cp437 && cat igclogo.ans | ./cp437 > igclogo4.ans
export GOPATH=/home/dan/git-remote                  &&  \
        cd /home/dan/git-remote/src/ansiart2utf8    &&  \
        go run ./ansiart2utf8.go -w 78 < ~/git-local/bbsd/data/igclogo4.ans > ~/git-local/bbsd/data/igclogo5.ans
cd ~/git-local/bbsd/data
cat igclogo5.ans | ./cp437 > igclogo6.ans

