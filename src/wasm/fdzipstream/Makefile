
# Build environment can be configured the following
# environment variables:
#   CC : Specify the C compiler to use
#   CFLAGS : Specify compiler options to use

CFLAGS += -Wall

all: zipexample zipfiles

zipexample: fdzipstream.h fdzipstream.c

zipfiles: fdzipstream.h fdzipstream.c

zipexample: fdzipstream.c zipexample.c
	$(CC) $(CFLAGS) -o zipexample fdzipstream.c zipexample.c -lz

zipfiles: fdzipstream.c zipfiles.c
	$(CC) $(CFLAGS) -o zipfiles fdzipstream.c zipfiles.c -lz 

clean:
	rm -f zipexample zipfiles

