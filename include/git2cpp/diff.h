#pragma once

#include <functional>

extern "C"
{
#include <git2/diff.h>
}

namespace git
{
   struct Repository;

   struct Diff
   {
      size_t deltas_num() const;

      void find_similar(git_diff_find_options & findopts)
      {
         git_diff_find_similar(diff_, &findopts);
      }

      Diff& merge(Diff const & other);

      enum class format
      {
         patch, patch_header, raw, name_only, name_status
      };

      typedef
         std::function<void (git_diff_delta const &, git_diff_hunk const &, git_diff_line const &)>
         print_callback_t;

      void print(format, print_callback_t print_callback) const;

      explicit Diff(git_diff * diff)
         : diff_(diff)
      {}

      ~Diff() { git_diff_free(diff_); }

      Diff              (Diff const &) = delete;
      Diff& operator =  (Diff const &) = delete;

      Diff(Diff && other)
         : diff_(other.diff_)
      {
         other.diff_ = nullptr;
      }

   private:
      git_diff * diff_;
   };

   struct Tree;

   Diff diff                   (Repository const &, Tree & a, Tree & b,  git_diff_options const &);
   Diff diff_to_index          (Repository const &, Tree &,              git_diff_options const &);
   Diff diff_index_to_workdir  (Repository const &,                      git_diff_options const &);
}

