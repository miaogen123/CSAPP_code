var_tiny=tiny.o open_listenfd.o RIO.o
tiny:$(var_tiny)
	cc -o tiny $(var_tiny)
tiny.o:tiny.c
	cc -c tiny.c
open_listenfd.o:open_listenfd.c
	cc -c open_listenfd.c
RIO.o:RIO.c
	cc -c RIO.c 
clean:
	rm -f *.o
