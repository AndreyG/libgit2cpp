bin/log: bin/log.o bin/git/diff_list.o
	g++ -lgit2 bin/log.o bin/git/diff_list.o -o bin/log

bin/log.o: src/log.cpp src/git/commit.h src/git/repo.h src/git/diff_list.h src/git/pathspec.h
	g++ -std=c++11 -g -o bin/log.o -c src/log.cpp

bin/git/diff_list.o: src/git/diff_list.cpp src/git/diff_list.h
	g++ -std=c++11 -o bin/git/diff_list.o -c src/git/diff_list.cpp
