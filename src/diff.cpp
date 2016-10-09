#include "git2cpp/diff.h"
#include "git2cpp/error.h"

#include <cassert>

#ifdef USE_BOOST
#include <boost/container/flat_map.hpp>
#include <boost/assign/list_of.hpp>
#else
#include <unordered_map>
#endif

namespace git
{
    namespace diff
    {
        namespace stats {
        namespace format
        {
            const type none             (GIT_DIFF_STATS_NONE);
            const type full             (GIT_DIFF_STATS_FULL);
            const type _short           (GIT_DIFF_STATS_SHORT);
            const type number           (GIT_DIFF_STATS_NUMBER);
            const type include_summary  (GIT_DIFF_STATS_INCLUDE_SUMMARY);
        }}

#ifdef USE_BOOST
        template<typename Key, typename Value>
        using map_container = boost::container::flat_map<Key, Value>;
#else
        struct EnumHash{
            template <typename T>
            std::size_t operator()(T t) const{
                return static_cast<std::size_t>(t);
            }
        };

        template<typename Key, typename Value>
        using map_container = std::unordered_map<Key, Value, EnumHash>;
#endif

        git_diff_format_t convert(format f)
        {
            static const map_container<format, git_diff_format_t> converter
                  = {
                { format::patch,        GIT_DIFF_FORMAT_PATCH           },
                { format::patch_header, GIT_DIFF_FORMAT_PATCH_HEADER    },
                { format::raw,          GIT_DIFF_FORMAT_RAW             },
                { format::name_only,    GIT_DIFF_FORMAT_NAME_ONLY       },
                { format::name_status,  GIT_DIFF_FORMAT_NAME_STATUS     }
                }
                  ;
            return converter.at(f);
        }
    }

    namespace
    {
        int apply_callback  ( git_diff_delta const * delta
                            , git_diff_hunk const * hunk
                            , git_diff_line const * line
                            , void * payload )
        {
            assert(delta);
            assert(line);
            auto cb = reinterpret_cast<Diff::print_callback_t const *>(payload);
            (*cb)(*delta, *hunk, *line);
            return 0;
        }
    }

    Diff::~Diff() { git_diff_free(diff_); }

    void Diff::find_similar(git_diff_find_options & findopts)
    {
        git_diff_find_similar(diff_, &findopts);
    }

    size_t Diff::deltas_num() const
    {
        return git_diff_num_deltas(diff_);
    }

    void Diff::print(diff::format f, print_callback_t print_callback) const
    {
       git_diff_print(diff_, convert(f), &apply_callback, &print_callback);
    }

    Diff::Stats Diff::stats() const
    {
        git_diff_stats * stats;
        if (git_diff_get_stats(&stats, diff_))
            throw error_t("git_diff_get_stats fail");
        else
            return Stats(stats);
    }

    Diff::Stats::~Stats()
    {
        git_diff_stats_free(stats_);
    }

    Buffer Diff::Stats::to_buf(diff::stats::format::type format, size_t width) const
    {
        git_buf buf = GIT_BUF_INIT_CONST(nullptr, 0);
        if (git_diff_stats_to_buf(&buf, stats_, git_diff_stats_format_t(format.value()), width))
            throw error_t("git_diff_stats_to_buf fail");
        else
            return Buffer(buf);
    }
}

