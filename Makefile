all: bin/log bin/branch

bin/log: bin/log.o bin/git/diff_list.o bin/git/repo.o
	g++ -lgit2 bin/log.o bin/git/repo.o bin/git/diff_list.o -o bin/log

bin/branch: bin/branch.o bin/git/repo.o
	g++ -lgit2 bin/branch.o bin/git/repo.o -o bin/branch

bin/log.o: src/log.cpp src/git/commit.h src/git/repo.h src/git/diff_list.h src/git/pathspec.h
	g++ -std=c++11 -g -o bin/log.o -c src/log.cpp

bin/branch.o: src/branch.cpp src/git/repo.h
	g++ -std=c++11 -g -o bin/branch.o -c src/branch.cpp

bin/git/diff_list.o: src/git/diff_list.cpp src/git/diff_list.h
	g++ -std=c++11 -o bin/git/diff_list.o -c src/git/diff_list.cpp

bin/git/repo.o: src/git/repo.cpp src/git/repo.h
	g++ -std=c++11 -o bin/git/repo.o -c src/git/repo.cpp
