#pragma once

#include "pathspec.h"

namespace git
{
    struct Repository;

    struct Tree
    {
        struct BorrowedEntry
        {
           explicit operator bool () const { return entry_; }
	   const char *    name()   const;
           git_oid const & id()     const;
	   git_otype       type() const;

           git_tree_entry const * ptr() const { return entry_; }

        private:
           friend struct Tree;

           explicit BorrowedEntry(git_tree_entry const * entry)
              : entry_(entry)
           {}

        private:
           git_tree_entry const * entry_;
        };

        struct OwnedEntry
        {
           ~OwnedEntry();

           OwnedEntry               (OwnedEntry const &) = delete;
           OwnedEntry&  operator =  (OwnedEntry const &) = delete;

           OwnedEntry               (OwnedEntry &&);
           OwnedEntry&  operator =  (OwnedEntry &&);

           Tree as_tree() /*&&*/;

        private:
            friend struct Tree;

            OwnedEntry(git_tree_entry * entry, Repository const & repo);

        private:
            git_tree_entry * entry_;
            Repository const * repo_;
        };

        git_tree const  * ptr() const   { return tree_; }
        git_tree        * ptr()         { return tree_; }

        int pathspec_match(uint32_t flags, Pathspec const & ps);

        size_t entrycount() const;

        BorrowedEntry operator[] (size_t) const;

        BorrowedEntry operator[] (std::string const & filename) const;

        OwnedEntry find(const char * path) const;

        Tree(git_oid const &, Repository const &);
        Tree(git_tree *,      Repository const &);

        Tree();
        ~Tree();

        Tree            (Tree &&);
        Tree& operator =(Tree &&);

        Tree            (Tree const &) = delete;
        Tree& operator =(Tree const &) = delete;

        explicit operator bool() const { return tree_; }

    private:
        git_tree * tree_;
        Repository const * repo_;
    };
}

