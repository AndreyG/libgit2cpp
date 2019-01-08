#pragma once

#include "pathspec.h"

namespace git
{
    struct Repository;

    struct Tree
    {
        struct BorrowedEntry
        {
            const char * name() const;
            git_oid const & id() const;

        private:
            friend struct Tree;
            friend struct Repository;

            explicit BorrowedEntry(git_tree_entry const * entry)
                : entry_(entry)
            {}

            git_tree_entry const * ptr() const { return entry_; }

        private:
            git_tree_entry const * entry_;
        };

        struct OwnedEntry
        {
            ~OwnedEntry();

            OwnedEntry(OwnedEntry const &) = delete;
            OwnedEntry & operator=(OwnedEntry const &) = delete;

            OwnedEntry(OwnedEntry &&) noexcept;
            OwnedEntry & operator=(OwnedEntry &&) noexcept;

            Tree to_tree() /*&&*/;

        private:
            friend struct Tree;
            friend struct Repository;

            OwnedEntry(git_tree_entry * entry, Repository const & repo);

            git_tree_entry const * ptr() const { return entry_; }

        private:
            git_tree_entry * entry_;
            Repository const * repo_;
        };

        git_tree const * ptr() const { return tree_; }
        git_tree * ptr() { return tree_; }

        int pathspec_match(uint32_t flags, Pathspec const & ps);

        size_t entrycount() const;

        BorrowedEntry operator[](size_t) const;

        BorrowedEntry operator[](std::string const & filename) const;

        OwnedEntry find(const char * path) const;

        Tree(git_tree *, Repository const &);

        Tree();
        ~Tree();

        Tree(Tree &&) noexcept;
        Tree & operator=(Tree &&) noexcept;

        Tree(Tree const &) = delete;
        Tree & operator=(Tree const &) = delete;

        explicit operator bool() const { return tree_ != nullptr; }

    private:
        git_tree * tree_;
        Repository const * repo_;
    };
}
