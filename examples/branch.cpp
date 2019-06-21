#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"

#include <cassert>
#include <iostream>

namespace
{
    using namespace git;

    int create_branch(Repository & repo, const char * name, bool force)
    {
        try
        {
            auto head = repo.head();
            auto branch = repo.create_branch(name, repo.commit_lookup(head.target()), force);
            assert(branch);
            assert(std::strcmp(branch.name(), name));
            std::cout << "branch " << name << " has been created" << std::endl;
            return EXIT_SUCCESS;
        }
        catch (branch_create_error const & e)
        {
            switch (e.reason)
            {
            case branch_create_error::already_exists:;
                std::cerr << "a branch named '" << name << "' already exists" << std::endl;
                break;
            case branch_create_error::invalid_spec:
                std::cerr << "'" << name << "' is not a valid branch name" << std::endl;
                break;
            case branch_create_error::unknown:
                break;
            }
            return EXIT_FAILURE;
        }
    }

    void print_help()
    {
        std::cerr   << "invalid command line arguments, expected are:" << std::endl
                    << "[[-f] branch_name]" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    Initializer threads_initializer;

    Repository repo(".");
    switch (argc)
    {
    case 1:
    {
        auto branches = repo.branches(git::branch_type::ALL);
        for (auto const & b : branches)
            std::cout << b.name() << std::endl;
        return EXIT_SUCCESS;
    }
    case 2:
        return create_branch(repo, argv[1], false);
    case 3:
        if (std::strcmp(argv[1], "-f") != 0)
        {
            print_help();
            return EXIT_FAILURE;
        }
        return create_branch(repo, argv[2], true);
    default:
        print_help();
        return EXIT_FAILURE;
    }
}
