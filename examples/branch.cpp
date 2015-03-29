#include <iostream>

#include "git2cpp/repo.h"

int main()
{
    git::Repository repo(".");
    auto branches = repo.branches(git::branch_type::LOCAL);
    for (auto b : branches)
        std::cout << b << std::endl;
}
