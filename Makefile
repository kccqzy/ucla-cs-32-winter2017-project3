CXX=/usr/local/opt/llvm/bin/clang++
CXXFLAGS=-Wall -Wextra -Wno-deprecated-declarations -O0 -fno-rtti -fno-exceptions -march=native -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -g -std=c++14 -stdlib=libc++ -MMD
VPATH=src

.PHONY: clean regen

regen: Bugs
	awk '{ print } /^# AUTOGENERATED/ { exit }' Makefile > Makefile.new
	cat *.d >> Makefile.new
	mv -f Makefile.new Makefile

clean:
	-rm Bugs *.o *.d

Bugs: Actor.o GameController.o GameWorld.o main.o StudentWorld.o
	$(CXX) $(CXXFLAGS) -framework OpenGL $^ /opt/X11/lib/libglut.dylib -o $@

# AUTOGENERATED DEPENDENCIES BELOW
Actor.o: src/Actor.cpp src/Actor.h src/Compiler.h src/GameConstants.h \
  src/GraphObject.h src/SpriteManager.h src/freeglut.h \
  src/freeglut_std.h src/freeglut_ext.h src/StudentWorld.h src/Field.h \
  src/GameWorld.h
GameController.o: src/GameController.cpp src/freeglut.h \
  src/freeglut_std.h src/freeglut_ext.h src/GameController.h \
  src/SpriteManager.h src/GameConstants.h src/GameWorld.h \
  src/GraphObject.h src/SoundFX.h
GameWorld.o: src/GameWorld.cpp src/GameWorld.h src/GameConstants.h \
  src/GameController.h src/SpriteManager.h src/freeglut.h \
  src/freeglut_std.h src/freeglut_ext.h
StudentWorld.o: src/StudentWorld.cpp src/StudentWorld.h src/Compiler.h \
  src/GameConstants.h src/Field.h src/GameWorld.h src/Actor.h \
  src/GraphObject.h src/SpriteManager.h src/freeglut.h \
  src/freeglut_std.h src/freeglut_ext.h
main.o: src/main.cpp src/GameController.h src/SpriteManager.h \
  src/freeglut.h src/freeglut_std.h src/freeglut_ext.h \
  src/GameConstants.h
