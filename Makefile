all: bin/log bin/branch bin/add

bin/log: bin/log.o bin/git/diff_list.o bin/git/repo.o bin/git/index.o
	g++ -lgit2 bin/log.o bin/git/repo.o bin/git/diff_list.o bin/git/index.o -o bin/log

bin/branch: bin/branch.o bin/git/repo.o
	g++ -lgit2 bin/branch.o bin/git/repo.o bin/git/index.o -o bin/branch

bin/add: bin/add.o bin/git/repo.o bin/git/index.o
	g++ -lgit2 bin/add.o bin/git/repo.o bin/git/index.o -o bin/add

bin/log.o: src/log.cpp src/git/commit.h src/git/repo.h src/git/diff_list.h src/git/pathspec.h
	g++ -std=c++11 -g -o bin/log.o -c src/log.cpp

bin/branch.o: src/branch.cpp src/git/repo.h
	g++ -std=c++11 -g -o bin/branch.o -c src/branch.cpp

bin/add.o: src/add.cpp src/git/repo.h
	g++ -std=c++11 -g -o bin/add.o -c src/add.cpp

bin/git/diff_list.o: src/git/diff_list.cpp src/git/diff_list.h
	g++ -std=c++11 -o bin/git/diff_list.o -c src/git/diff_list.cpp

bin/git/repo.o: src/git/repo.cpp src/git/repo.h
	g++ -std=c++11 -o bin/git/repo.o -c src/git/repo.cpp

bin/git/index.o: src/git/index.cpp src/git/index.h
	g++ -std=c++11 -o bin/git/index.o -c src/git/index.cpp
