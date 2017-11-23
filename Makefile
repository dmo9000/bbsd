OBJS = main.o pipeline.o subprocess.o nvt.o
#FLAGS = -g -ggdb -std=c++11
FLAGS = -std=c++11



all: pmain mainmenu

mainmenu: mainmenu.o pipeline.o subprocess.o
	g++ $(FLAGS) -o $@ mainmenu.o pipeline.o subprocess.o

pmain:	$(OBJS)	
	g++ $(FLAGS) -o $@ $(OBJS)

opencommand:	opencommand.o
	gcc -o opencommand opencommand.o


%.o: %.cpp
	g++ -c $(CXX_FLAGS) $(FLAGS) -o $@ $<

clean:
	rm -f pmain open2 mainmenu *.o

install:
	mkdir -p /usr/local/bbsd/data
	cp pmain /usr/local/bbsd
	cp mainmenu /usr/local/bbsd

	
