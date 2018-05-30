CXX = g++
CC = gcc

OBJS =pipeline.o subprocess.o nvt.o
FLAGS = -std=c++11 -D__LINUX__ -fpermissive -g -ggdb
LDFLAGS = -static

all: buildtag mainmenu keystrokes pmain 

buildtag:
	./build-id.sh && make clean

mainmenu: mainmenu.o $(OBJS) 
	g++ $(LDFLAGS) $(FLAGS) -o $@ mainmenu.o $(OBJS) 

keystrokes: keystrokes.o $(OBJS) 
	g++ $(LDFLAGS) $(FLAGS) -o $@ keystrokes.o $(OBJS) 

pmain: main.o $(OBJS)	
	g++ $(LDFLAGS) $(FLAGS) -o $@ main.o $(OBJS)

%.o: %.cpp
	g++ -c $(CXX_FLAGS) $(FLAGS) -o $@ $<

docker: install
	rm -rf dockerdata/usr
	mkdir -p dockerdata/usr/bin
	mkdir -p dockerdata/usr/local/bbsd
	cp -rfpv /usr/bin/tdftool dockerdata/usr/bin/tdftool
	cp -rfpv /usr/local/bbsd/* dockerdata/usr/local/bbsd
	docker build -t bbsd .

clean:
	rm -f pmain mainmenu keystrokes *.o 
	rm -rf dockerdata/usr

install:
	sudo systemctl stop bbsd 
	sudo mkdir -p /usr/local/bbsd/data
	sudo mkdir -p /usr/local/bbsd/fonts
	sudo cp data/* /usr/local/bbsd/data
	sudo cp fonts/* /usr/local/bbsd/fonts
	sudo cp pmain /usr/local/bbsd
	sudo cp mainmenu /usr/local/bbsd
	sudo cp keystrokes /usr/local/bbsd
	sudo chown -R nobody:nobody /usr/local/bbsd
	sudo rm -f /etc/systemd/system/multi-user.target.wants/bbsd.service
	sudo systemctl enable bbsd.service
	sudo systemctl daemon-reload
	sudo service bbsd start
	sudo service bbsd status
	
	
