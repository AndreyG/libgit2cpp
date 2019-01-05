#pragma once

struct git_signature;
struct git_repository;

namespace git
{
    struct Signature
    {
        git_signature const * ptr() const { return sig_; }

        explicit Signature(git_repository * repo);
        Signature(const char * name, const char * email, git_time_t time, int offset);

        ~Signature();

    private:
        git_signature * sig_;
    };
}
