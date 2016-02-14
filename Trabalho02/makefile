CC = gcc
CFLAGS = -Wall -lpthread
CFLAGS2 = -Wall
MAIN = main.out
TESTE = testGenerator.out
SRCS = elevator.c main.c
SRCS2 = testGenerator.c
OBJS = $(SRCS:.c=.o)
OBJS2 = $(SRCS2:.c=.o)

.PHONY: depend clean test

all: $(MAIN)
	@echo "Program has been compiled using $(CC) compiler"

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) */*.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

test: $(SRCS2)
	@echo "Compiling test generator"
	$(CC) $(CFLAGS2) $(INCLUDES) -c $< -o $(TESTE)
