libgit2cpp
==========

C++ wrapper for libgit2. It contains libgit2 as submodule (libs/libgit2).

Building libgit2cpp - Using CMake
=================================

    $ mkdir build && cd build
    $ cmake ..
    $ make
    
Supporting CMake options: `USE_BOOST`, `BUNDLE_LIBGIT2`.

Testing 
=======

    $ cd build
    $ ./test.sh <path_to_non_shared_repo> [path_to_libgit2_testrepo]

e.g.
    
    $ mkdir build && cd build
    $ cmake ..
    $ make
    $ ./test.sh .. ../libs/libgit2/tests/resources/testrepo.git
