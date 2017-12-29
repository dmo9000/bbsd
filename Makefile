OBJS =pipeline.o subprocess.o nvt.o
FLAGS = -std=c++11 -D__LINUX__ -fpermissive -g -ggdb

all: buildtag mainmenu pmain

buildtag:
	./build-id.sh && make clean

mainmenu: mainmenu.o $(OBJS) 
	g++ $(FLAGS) -o $@ mainmenu.o $(OBJS) 

pmain: main.o $(OBJS)	
	g++ $(FLAGS) -o $@ main.o $(OBJS)

%.o: %.cpp
	g++ -c $(CXX_FLAGS) $(FLAGS) -o $@ $<

clean:
	rm -f pmain mainmenu *.o

install:
	sudo systemctl stop bbsd 
	sudo mkdir -p /usr/local/bbsd/data
	sudo cp data/* /usr/local/bbsd/data
	sudo cp pmain /usr/local/bbsd
	sudo cp mainmenu /usr/local/bbsd
	#cp systemd/bbsd.service /lib/systemd/system
	sudo cp systemd/bbsd.service \
		/etc/systemd/system/multi-user.target.wants/bbsd.service
	sudo systemctl enable bbsd.service
	sudo systemctl daemon-reload
	sudo service bbsd start
	sudo service bbsd status
	
