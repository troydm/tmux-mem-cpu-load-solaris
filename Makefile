CC=gcc

make:
	$(CC) -lkstat ./tmux-mem-cpu-load-solaris.c -o tmux-mem-cpu-load

install:
	cp ./tmux-mem-cpu-load /usr/local/bin/

clean:
	rm -f ./tmux-mem-cpu-load > /dev/null
