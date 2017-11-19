OBJS = main.o pipeline.o subprocess.o nvt.o
FLAGS = -g -ggdb

main:	$(OBJS)	
	g++ $(FLAGS) -o $@ $(OBJS)

%.o: %.cpp
	g++ -c $(CXX_FLAGS) $(FLAGS) -o $@ $<

clean:
	rm -f main *.o
