OBJS = main.o pipeline.o subprocess.o nvt.o
FLAGS = -g -ggdb


all: opencommand pmain

pmain:	$(OBJS)	
	g++ $(FLAGS) -o $@ $(OBJS)

opencommand:	opencommand.o
	gcc -o opencommand opencommand.o


%.o: %.cpp
	g++ -c $(CXX_FLAGS) $(FLAGS) -o $@ $<

clean:
	rm -f pmain open2 opencommand *.o
