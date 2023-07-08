OBJDIR	 = ./build
OBJECTS	 = $(OBJDIR)/host.o $(OBJDIR)/slave.o
SRCDIR   = ./src
SWCC     = swgcc

all: host.o slave.o a.out

a.out: $(OBJECTS) | dir
	$(SWCC) -mhybrid  $^ -o $(OBJDIR)/a.out

host.o: $(SRCDIR)/host.c | dir
	$(SWCC) -mhost -msimd -c  $< -o $(OBJDIR)/host.o

slave.o: $(SRCDIR)/slave.c | dir
	$(SWCC) -mslave -c  $< -o $(OBJDIR)/slave.o

dir:
	@mkdir -p $(OBJDIR)

.PHONY: cl

cl:
	rm -rf ./build