APP = sserver
CC=gcc
$(APP): *.o *.h
	$(CC) *.o -o $(APP)
.c.o:
	$(CC) -c -g $<
