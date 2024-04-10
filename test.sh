#!/bin/bash

#trace
# set -x
#verbose outputs each input line as is read
# set -v

if [ $# -eq 0 ] ; then
    echo "usage: test.sh read_only_repo [writable_repo]" 1>&2
	return 1
fi

REPO=$1

if [ ! -d $REPO ]; then
    echo "repo $REPO not a directory" 1>&2
	exit 1
fi

CWD=${PWD}

function test()
{
    local test_name="$1"
    echo -e "**** test $test_name *********************************\n\n"
    local bin="$CWD/examples/$test_name"
    
    shift
    
    $bin "$@"
    
    local exit_status
    
    exit_status=$?
    
    echo -e "\nexit status $exit_status \n\n"
}

# read-only tests (repo shouldn't be bare)
pushd $REPO

test branch-cpp
test diff-cpp
test commit-graph-generator-cpp . "$CWD/commit-graph.dot"
test log-cpp
test rev-list-cpp --topo-order HEAD
test rev-parse-cpp HEAD
test showindex-cpp
test status-cpp

popd

# write test (use libgit2/tests/resources/testrepo.git)

RW_REPO=$2

if [ -z $RW_REPO ]; then
	echo "no writable repo specified"
	exit 0
fi

pushd $RW_REPO

test cat-file-cpp -t a8233120f6ad708f843d861ce2b7228ec4e3dec6
test cat-file-cpp -p a8233120f6ad708f843d861ce2b7228ec4e3dec6
test general-cpp .

popd
