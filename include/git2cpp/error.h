#pragma once

#include <stdexcept>
#include <sstream>

#include "id_to_str.h"

namespace git
{
    struct repository_open_error : std::exception
    {
        virtual const char * what() const noexcept override 
        {
            return "Could not open repository";
        }
    };

    struct repository_init_error : std::exception
    {
        explicit repository_init_error(std::string const & dir)
            : message_("Could not initialize repository on path " + dir)
        {}

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct index_open_error : std::exception
    {
        virtual const char * what() const noexcept override 
        {
            return "Could not open repository index";
        }
    };

    struct odb_open_error : std::exception
    {
        virtual const char * what() const noexcept override 
        {
            return "Could not open ODB";
        }
    };

    struct commit_lookup_error : std::exception
    {
        explicit commit_lookup_error(git_oid const * id)
            : message_("Could not lookup commit " + id_to_str(id)) 
        {}

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct tree_lookup_error : std::exception
    {
        explicit tree_lookup_error(git_oid const * id)
            : message_("Could not lookup tree " + id_to_str(id)) 
        {}

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct tag_lookup_error : std::exception
    {
        explicit tag_lookup_error(git_oid const * id)
            : message_("Could not lookup tag " + id_to_str(id))
        {}

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct commit_parent_error : std::exception
    {
        explicit commit_parent_error(git_oid const * id)
            : message_("Could not get parent for commit " + id_to_str(id)) 
        {}

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct commit_tree_error : std::exception
    {
        explicit commit_tree_error(git_oid const * id)
            : message_("Could not get tree for commit " + id_to_str(id)) 
        {}

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct revparse_error : std::exception
    {
        explicit revparse_error(const char * spec)
        {
            std::ostringstream ss;
            ss << "Could not resolve " << spec;
            message_ = ss.str();
        }

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct odb_read_error : std::exception
    {
        explicit odb_read_error(git_oid const * id)
            : message_("Could not find obj " + id_to_str(id)) 
        {}

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct odb_write_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
            return "odb write error";
        }
    };

    struct file_not_found_error : std::exception
    {
        explicit file_not_found_error(const char * filepath)
        {
            std::ostringstream ss;
            ss << "file path \"" << filepath << "\" not found";
            message_ = ss.str();
        }

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct ambiguous_path_error : std::exception
    {
        explicit ambiguous_path_error(const char * filepath)
        {
            std::ostringstream ss;
            ss << "file path \"" << filepath << "\" is ambiguous";
            message_ = ss.str();
        }

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct unknown_file_status_error : std::exception
    {
        explicit unknown_file_status_error(const char * filepath)
        {
            std::ostringstream ss;
            ss << "unknown error during getting status for file \"" << filepath;
            message_ = ss.str();
        }

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    struct pathspec_new_error : std::exception
    {
        virtual const char * what() const noexcept override 
        {
            return "Could not build pathspec";
        }
    };

    struct revwalk_new_error : std::exception
    {
        virtual const char * what() const noexcept override 
        {
            return "Could not create revision walker";
        }
    };

    struct invalid_head_error : std::exception
    {
        virtual const char * what() const noexcept override 
        {
            return "Could not find repository HEAD";
        }
    };

    struct non_commit_object_error : std::exception
    {
        explicit non_commit_object_error(git_oid const * id)
        {
            std::ostringstream ss;
            ss << "object " << id_to_str(id) << " is not a commit";
            message_ = ss.str();
        }

        virtual const char * what() const noexcept override
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };
    
    struct unknown_get_current_branch_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
            return "failed to get current branch"; 
        }
    };

    struct get_status_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
            return "Could not get status"; 
        }
    };

    struct signature_create_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
		    return  "Unable to create a commit signature."
                    " Perhaps 'user.name' and 'user.email' are not set";
        }
    };

    struct index_write_tree_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
            return "Unable to write tree from index";
        }
    };

    struct index_write_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
            return "Unable to write index";
        }
    };

    struct commit_create_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
		    return "Could not create commit"; 
        }
    };

    struct merge_base_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
		    return "Could not find merge base"; 
        }
    };

    struct config_open_error : std::exception
    {
        virtual const char * what() const noexcept override
        {
            return "Could not open config";
        }
    };
}

