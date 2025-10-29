#Mouse Injector Makefile
#Use with TDM-GCC 4.9.2-tdm-3 or MinGW
#For portable MinGW download Orwell Dev-C++
#mingw32-make.exe -f makefile to compile

#Compiler directories
#MINGWDIR = C:/Dev/Dev-Cpp/MinGW64/bin/
MINGWDIR = C:/TDM-GCC-64/bin/
CC = $(MINGWDIR)gcc
WINDRES = $(MINGWDIR)windres

#Source directories
SRCDIR = ./
MANYMOUSEDIR = $(SRCDIR)manymouse/
SDLDIR = $(SRCDIR)include/SDL2/
GAMESDIR = $(SRCDIR)games/
OBJDIR = $(SRCDIR)obj/
EXENAME = "$(SRCDIR)Mouse Injector.exe"

#Compiler flags
CFLAGS = -ansi -O2 -m64 -std=c99 -Wall
WFLAGS = -Wextra -pedantic -Wno-parentheses
RESFLAGS = -F pe-x86-64 --input-format=rc -O coff

#Linker flags
OBJS = $(OBJDIR)main.o $(OBJDIR)memory.o $(OBJDIR)mouse.o $(OBJDIR)manymouse.o $(OBJDIR)windows_wminput.o $(OBJDIR)icon.res $(OBJDIR)joystick.o  #$(OBJDIR)xinputjoy.o 
GAMEOBJS = $(patsubst $(GAMESDIR)%.c, $(OBJDIR)%.o, $(wildcard $(GAMESDIR)*.c))
LIBS = -static-libgcc -lpsapi -lwinmm  -lmingw32 -lSDL2main -lSDL2 #-lxinput
LFLAGS = $(OBJS) $(GAMEOBJS) -I/include -L/lib -o $(EXENAME) $(LIBS) -m64 -s

#Main recipes
mouseinjector: $(OBJS) $(GAMEOBJS)
	$(CC) $(LFLAGS)

all: clean mouseinjector

#Individual recipes
$(OBJDIR)main.o: $(SRCDIR)main.c $(SRCDIR)main.h $(SRCDIR)memory.h $(SRCDIR)joystick.h $(SRCDIR)mouse.h $(GAMESDIR)game.h
	$(CC) -c $(SRCDIR)main.c -o $(OBJDIR)main.o $(CFLAGS) $(WFLAGS)

$(OBJDIR)memory.o: $(SRCDIR)memory.c $(SRCDIR)memory.h
	$(CC) -c $(SRCDIR)memory.c -o $(OBJDIR)memory.o $(CFLAGS) $(WFLAGS)

$(OBJDIR)joystick.o: $(SRCDIR)joystick.c $(SRCDIR)joystick.h 
	$(CC) -c $(SRCDIR)joystick.c -o $(OBJDIR)joystick.o $(CFLAGS) $(WFLAGS)

# $(OBJDIR)xinputjoy.o: $(SRCDIR)xinputjoy.c $(SRCDIR)xinputjoy.h 
# 	$(CC) -c $(SRCDIR)xinputjoy.c -o $(OBJDIR)xinputjoy.o $(CFLAGS) $(WFLAGS)

$(OBJDIR)mouse.o: $(SRCDIR)mouse.c $(SRCDIR)mouse.h $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(SRCDIR)mouse.c -o $(OBJDIR)mouse.o $(CFLAGS) $(WFLAGS)

$(OBJDIR)manymouse.o: $(MANYMOUSEDIR)manymouse.c $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(MANYMOUSEDIR)manymouse.c -o $(OBJDIR)manymouse.o $(CFLAGS) $(WFLAGS)

$(OBJDIR)windows_wminput.o: $(MANYMOUSEDIR)windows_wminput.c $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(MANYMOUSEDIR)windows_wminput.c -o $(OBJDIR)windows_wminput.o $(CFLAGS)


	
$(OBJDIR)icon.res: $(SRCDIR)icon.rc $(SRCDIR)icon.ico
	$(WINDRES) -i $(SRCDIR)icon.rc -o $(OBJDIR)icon.res $(RESFLAGS)

#Game drivers recipe
$(OBJDIR)%.o: $(GAMESDIR)%.c $(SRCDIR)main.h $(SRCDIR)memory.h $(SRCDIR)mouse.h $(GAMESDIR)game.h $(SRCDIR)joystick.h #$(SRCDIR)xinputjoy.h  
	$(CC) -c $< -o $@ $(CFLAGS) $(WFLAGS)

clean:
	rm -f $(SRCDIR)*.exe $(OBJDIR)*.o $(OBJDIR)*.res $(SRCDIR)*.ini