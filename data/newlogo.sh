#!/bin/sh

PATH=$PATH:$HOME/.local/bin:$HOME/bin
PATH=~/.npm-global/bin:~/.cargo/bin:$PATH
export PATH

export GOPATH=/home/dan/git-remote

~/.npm-global/bin/cfonts \
    --align center -f block "IGC|SOFTWARE" -c blue,yellow   | \
    ./cp437 2>/dev/null | \
    go run /home/dan/git-remote/src/ansiart2utf8/ansiart2utf8.go -w 78 - | \
    ./cp437 2>/dev/null > igclogo6.ans 

oozz gunk > gunk1.ans
./cp437 < gunk1.ans 2>/dev/null > gunk2.ans
go run /home/dan/git-remote/src/ansiart2utf8/ansiart2utf8.go -w 78 < gunk2.ans > gunk3.ans
./cp437 < gunk3.ans 2>/dev/null | head -n 22 > gunk4.ans
# trim the last line since it is incomplete
#sed -i '$ d' gunk4.ans
rm -f gunk[123].ans

go run /home/dan/git-remote/src/ansiart2utf8/ansiart2utf8.go -w 80 < bmilogo.ans > bmilogo2.ans
./cp437 < bmilogo2.ans 2>/dev/null > bmilogo3.ans
rm -f bmilogo[2].ans

