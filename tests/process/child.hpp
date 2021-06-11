#ifndef PROCESS_CHILD_HPP
#define PROCESS_CHILD_HPP

#include "child_decl.hpp"
#include "executor_impl.hpp"

namespace process {

    template <typename... Args>
    child::child(Args &&...args)
        : child(details::execute_impl(std::forward<Args>(args)...)) {
    }

} // namespace process

#endif // PROCESS_CHILD_HPP
