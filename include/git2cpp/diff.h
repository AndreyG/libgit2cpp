#pragma once

#include <cstdint>
#include <functional>

extern "C"
{
#include <git2/diff.h>
}

namespace git
{
   struct Repository;
   struct Tree;

   struct Diff
   {
      size_t deltas_num() const;

      Diff& merge(Diff const & other);

      enum class format
      {
         patch, patch_header, raw, name_only, name_status
      };

      enum class option : uint32_t
      {
         normal			= GIT_DIFF_NORMAL,
	 reverse		= GIT_DIFF_REVERSE,
	 include_unmodified	= GIT_DIFF_INCLUDE_UNMODIFIED,
	 include_typechange	= GIT_DIFF_INCLUDE_TYPECHANGE,
	 ignore_filemode	= GIT_DIFF_IGNORE_FILEMODE,
	 ignore_submodules	= GIT_DIFF_IGNORE_SUBMODULES,
      };

      friend option operator ~ (option);
      friend option operator | (option, option);
      friend option operator & (option, option);
      
      enum class find_option : uint32_t
      {
         none			= GIT_DIFF_FIND_BY_CONFIG,
	 renames		= GIT_DIFF_FIND_RENAMES,
	 copies			= GIT_DIFF_FIND_COPIES,
	 ignore_whitespace	= GIT_DIFF_FIND_IGNORE_WHITESPACE,
	 exact_match_only	= GIT_DIFF_FIND_EXACT_MATCH_ONLY,
      };

      friend find_option operator ~ (find_option);
      friend find_option operator | (find_option, find_option);
      friend find_option operator & (find_option, find_option);
      
      typedef
          std::function<void (git_diff_delta const &, const git_diff_hunk *, git_diff_line const &)>
          print_callback_t;

      void find_similar(find_option findopts);

      void print(format, print_callback_t print_callback) const;

      explicit Diff(git_diff * diff)
         : diff_(diff)
      {}
      
      explicit Diff(Repository const &repo, Tree & a, Tree & b, option const &optmask);
      
      static Diff diff_to_index(Repository const &repo, Tree &t, option const &optmask);
      static Diff diff_index_to_workdir(Repository const &repo, option const &optmask);

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
}

