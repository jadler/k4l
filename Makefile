########## Makefile ##########
TARGET		=	libnui.so
CXX		=	g++
CC		=	gcc
CFLAGS		=	-Wall -ansi -g -fPIC
LIBS		=	`pkg-config --libs opencv` `pkg-config --libs libfreenect` `pkg-config --libs lua5.1` -lpthread

SRCDIR		=	src
OBJDIR		=	obj
BINDIR		=	bin

LIBDIR		=	/usr/lib/lua/5.1

SRC=$(wildcard $(SRCDIR)/*.c*)
OBJ+=$(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(wildcard *.c)))
OBJ+=$(addprefix $(OBJDIR)/,$(patsubst %.cpp,%.o,$(wildcard *.cpp)))

$(TARGET): $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -o $(OBJDIR)/kinect.o -c $(SRCDIR)/kinect.cpp
	$(CC) $(CFLAGS) -o $(OBJDIR)/wrapper.o -c $(SRCDIR)/wrapper.c
	$(CC) $(CFLAGS) -shared -o $(BINDIR)/$@ $(LIBS) $(OBJDIR)/kinect.o $(OBJDIR)/wrapper.o

#$(BINDIR)/$(TARGET): $(OBJDIR) $(BINDIR) $(OBJ)
#	$(CXX) $(CFLAGS) -shared -o $@ $(LIBS) $(OBJ)

#$(OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%
#	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR) $(OBJDIR):
	mkdir -p $@

install:
	sudo cp $(BINDIR)/$(TARGET) $(LIBDIR)

.PHONY: remove clean
remove:
	rm -rf $(OBJDIR)

clean: remove
	rm -rf $(BINDIR)
	sudo rm $(LIBDIR)/$(TARGET)
