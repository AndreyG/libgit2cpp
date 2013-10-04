#include <iostream>

#include "git2cpp/repo.h"

int main()
{
    git::Repository repo(".");
    auto branches = repo.branches();
    for (auto b : branches)
        std::cout << b << std::endl;
}
