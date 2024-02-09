PROGRAMS:= sshell
CC:= gcc 
CFLAGS:= -Wall -Werror -Wextra

all: $(PROGRAMS)

sshell: sshell.c
	$(CC) $(CFLAGS) -g $^ -o $@
.PHONY: clean
clean:
	rm $(PROGRAMS)
	
