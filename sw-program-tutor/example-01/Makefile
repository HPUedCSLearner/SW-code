OBJDIR	 = ./build
OBJECTS	 = $(OBJDIR)/host.o $(OBJDIR)/slave.o $(OBJDIR)/main.o
SRCDIR   = ./src
SWCC     = swgcc
FLAS     = -O3

all: main.o host.o slave.o a.out

a.out: $(OBJECTS) | dir
	$(SWCC) $(FLAS) -mhybrid  $^ -o $(OBJDIR)/a.out

host.o: $(SRCDIR)/host.c | dir
	$(SWCC) $(FLAS) -mhost -c  $< -o $(OBJDIR)/host.o

main.o: $(SRCDIR)/main.c | dir
	$(SWCC) $(FLAS) -mhost -c  $< -o $(OBJDIR)/main.o

slave.o: $(SRCDIR)/slave.c | dir
	$(SWCC) $(FLAS) -mslave -msimd -c  $< -o $(OBJDIR)/slave.o

dir:
	@mkdir -p $(OBJDIR)

.PHONY: cl

cl:
	rm -rf ./build