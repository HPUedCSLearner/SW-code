OBJDIR	 = ./build
OBJECTS	 = $(OBJDIR)/main.o $(OBJDIR)/master.o $(OBJDIR)/slave.o
SRCDIR   = ./src
SWCC     = swgcc
FLAS     = -O2

all: main.o master.o slave.o a.out

a.out: $(OBJECTS) | dir
	$(SWCC) $(FLAS) -mhybrid -faddress_align=64  $^ -o $(OBJDIR)/a.out

main.o: $(SRCDIR)/main.c | dir
	$(SWCC) $(FLAS) -mhost -faddress_align=32 -c  $< -o $(OBJDIR)/main.o

master.o: $(SRCDIR)/master.c | dir
	$(SWCC) $(FLAS) -mhost -faddress_align=32 -c  $< -o $(OBJDIR)/master.o

slave.o: $(SRCDIR)/slave.c | dir
	$(SWCC) $(FLAS) -mslave -msimd -faddress_align=64 -c  $< -o $(OBJDIR)/slave.o

dir:
	@mkdir -p $(OBJDIR)

.PHONY: cl

cl:
	rm -rf ./build