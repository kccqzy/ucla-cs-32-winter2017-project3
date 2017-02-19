CXX=/usr/local/opt/llvm/bin/clang++
CXXFLAGS=-Wall -Wextra -Wno-deprecated-declarations -O1 -fno-rtti -fno-exceptions -march=native -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -g -std=c++14 -stdlib=libc++
VPATH=src

Bugs: Actor.o GameController.o GameWorld.o main.o StudentWorld.o
	$(CXX) $(CXXFLAGS) -framework OpenGL $^ /opt/X11/lib/libglut.dylib -o $@

.PHONY: clean
clean:
	-rm Bugs *.o

Actor.o: Actor.cpp Actor.h StudentWorld.h

GameController.o: GameController.cpp freeglut.h GameController.h GameWorld.h GameConstants.h GraphObject.h SoundFX.h SpriteManager.h

GameWorld.o: GameWorld.h GameWorld.cpp GameController.h

main.o: main.cpp GameController.h

StudentWorld.o: StudentWorld.h StudentWorld.cpp
