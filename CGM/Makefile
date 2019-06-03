CFLAGS= -pthread -lrt -O2
TARGETS= CGM.c periodic.h tasks.h
make: $(TARGETS)
	gcc $(TARGETS) -o cgm $(CFLAGS)
clean:
	rm -f cgm *.o