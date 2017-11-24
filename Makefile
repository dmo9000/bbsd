OBJS = main.o pipeline.o subprocess.o nvt.o
FLAGS = -std=c++11 -D__LINUX__ -fpermissive

all: mainmenu pmain

mainmenu: mainmenu.o pipeline.o subprocess.o
	g++ $(FLAGS) -o $@ mainmenu.o pipeline.o subprocess.o

pmain:	$(OBJS)	
	g++ $(FLAGS) -o $@ $(OBJS)

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
	
