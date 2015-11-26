#!/bin/bash

#trace
# set -x
#verbose outputs each input line as is read
# set -v

if [ $# -eq 0 ] ; then
    echo "usage: test.sh <repo>" 1>&2
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
    local bin="$CWD/$test_name"
    
    shift
    
    $bin "$@"
    
    local exit_status
    
    exit_status=$?
    
    echo -e "\nexit status $exit_status \n\n"
}

# read-only tests (repo shouldn't be bare)
pushd $REPO

test branch
test commit-graph-generator . "$CWD/commit-graph.dot"
test diff --patch HEAD HEAD~1
test diff --name-only HEAD HEAD~1
test log -3 --name-only
test log -3 --patch
test rev-list --topo-order HEAD
test rev-parse HEAD
test showindex
test status

popd

# tests on static repo

test_repo=resources/testrepo.git

pushd $test_repo

test cat-file -t a8233120f6ad708f843d861ce2b7228ec4e3dec6
test cat-file -p a8233120f6ad708f843d861ce2b7228ec4e3dec6
test general .

popd
