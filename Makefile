CXX = g++
CC = gcc

OBJS =pipeline.o subprocess.o nvt.o
FLAGS = -std=c++11 -D__LINUX__ -fpermissive -g -ggdb
LDFLAGS = -static

all: buildtag mainmenu pmain 

buildtag:
	./build-id.sh && make clean

mainmenu: mainmenu.o $(OBJS) 
	g++ $(LDFLAGS) $(FLAGS) -o $@ mainmenu.o $(OBJS) 

pmain: main.o $(OBJS)	
	g++ $(LDFLAGS) $(FLAGS) -o $@ main.o $(OBJS)

%.o: %.cpp
	g++ -c $(CXX_FLAGS) $(FLAGS) -o $@ $<

docker:
	docker build -t bbsd .

clean:
	rm -f pmain mainmenu

install:
	sudo systemctl stop bbsd 
	sudo mkdir -p /usr/local/bbsd/data
	sudo mkdir -p /usr/local/bbsd/fonts
	sudo cp data/* /usr/local/bbsd/data
	sudo cp fonts/* /usr/local/bbsd/fonts
	sudo cp pmain /usr/local/bbsd
	sudo cp mainmenu /usr/local/bbsd
	sudo chown -R nobody:nobody /usr/local/bbsd
#	sudo cp systemd/bbsd.service \
		/etc/systemd/system/multi-user.target.wants/bbsd.service
	sudo rm -f /etc/systemd/system/multi-user.target.wants/bbsd.service
	sudo systemctl enable bbsd.service
	sudo systemctl daemon-reload
	sudo service bbsd start
	sudo service bbsd status
	
	
