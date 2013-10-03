#pragma once

#include <stdexcept>
#include <sstream>

namespace git
{
    struct repository_open_error : std::exception
    {
        virtual const char * what() const noexcept override 
        {
            return "Could not open repository";
        }
    };

    struct index_open_error : std::exception
    {
        virtual const char * what() const noexcept override 
        {
            return "Could not open repository index";
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
}

