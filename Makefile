CC="gcc"
CFLAGS="-Wall"

all:
	$(CC) -o ppm_test $(CFLAGS) ppm_test.c

clean:
	rm -f ppm_test

install: all
	cp ppm_test /usr/local/bin/ppm_test

uninstall:
	rm -f /usr/local/bin/ppm_test
