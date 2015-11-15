# TODO: As it stands right now, this file is really nothing more than a silly
# shell script; it should probably be revisited and improved at some point.

build:
	@cd ./build && /usr/local/Cellar/qt5/5.5.0/bin/qmake && make

clean:
	@rm -f ./build/koala
	@rm -f ./build/Makefile
	@rm -f ./build/.qmake.stash
	@rm -f ./build/*.cpp
	@rm -f ./build/*.o

.PHONY: build clean
