#pragma once

struct git_signature;
struct git_repository;

namespace git
{
    struct Signature
    {
        git_signature const * ptr() const { return sig_; }

        explicit Signature(git_repository * repo);
        ~Signature();

    private:
        git_signature * sig_;
    };
}

