OBJS = main.o pipeline.o subprocess.o nvt.o
FLAGS = -std=c++11 -DBBSD_ROOT="/usr/local/bbsd"


mainmenu: mainmenu.o pipeline.o subprocess.o
	g++ $(FLAGS) -o $@ mainmenu.o pipeline.o subprocess.o

pmain:	$(OBJS)	
	g++ $(FLAGS) -o $@ $(OBJS)

%.o: %.cpp
	g++ -c $(CXX_FLAGS) $(FLAGS) -o $@ $<

clean:
	rm -f pmain mainmenu *.o

install:
	mkdir -p /usr/local/bbsd/data
	cp pmain /usr/local/bbsd
	cp mainmenu /usr/local/bbsd
	cp systemd/bbsd.service /lib/systemd/system

	
