#ifndef PROCESS_EXCEPTION_HPP
#define PROCESS_EXCEPTION_HPP

#include <system_error>

namespace process {

    struct process_error : std::system_error {
        using std::system_error::system_error;
    };

} // namespace process

#endif // PROCESS_EXCEPTION_HPP
