libgit2cpp
=======

C++ wrapper for libgit2

Building libgit2cpp - Using CMake
==============================

    $ mkdir build && cd build
    $ cmake ..
    $ make

Testing 
=======

    $ cd build
    $ ./test.sh <path_to_non_shared_repo> [path_to_libgit2_testrepo]

e.g.
    
    $ mkdir build && cd build
    $ cmake ..
    $ make
    $ ./test.sh .. ../../libgit2/tests/resources/testrepo.git

