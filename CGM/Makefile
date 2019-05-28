CFLAGS= -Wall -pthread -lrt
TARGETS= CGM.c periodic.c 
make: CGM.c periodic.c 
	gcc $(TARGETS) -o cgm $(CFLAGS)
clean:
	rm cgm *.o