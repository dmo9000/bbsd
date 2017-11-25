OBJS =pipeline.o subprocess.o nvt.o
FLAGS = -std=c++11 -D__LINUX__ -fpermissive -g -ggdb

all: buildtag mainmenu pmain

buildtag:
	./build-id.sh

mainmenu: mainmenu.o $(OBJS) 
	g++ $(FLAGS) -o $@ mainmenu.o $(OBJS) 

pmain: main.o $(OBJS)	
	g++ $(FLAGS) -o $@ main.o $(OBJS)

%.o: %.cpp
	g++ -c $(CXX_FLAGS) $(FLAGS) -o $@ $<

clean:
	rm -f pmain mainmenu *.o

install:
	sudo service bbsd stop 
	mkdir -p /usr/local/bbsd/data
	cp data/* /usr/local/bbsd/data
	cp pmain /usr/local/bbsd
	cp mainmenu /usr/local/bbsd
	cp systemd/bbsd.service /lib/systemd/system
	sudo service bbsd start
	sudo service bbsd status
	
