CFLAGS= -pthread -lrt -O2
TARGETS= CGM.c periodic.h tasks.h
TARGETS2= controller.c
make: $(TARGETS) $(TARGETS2)
	gcc $(TARGETS) -o cgm $(CFLAGS)
	gcc $(TARGETS2) -o ctrl $(CFLAGS)
clean:
	rm -f cgm ctrl *.o