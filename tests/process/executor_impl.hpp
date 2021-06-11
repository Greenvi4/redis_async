#ifndef PROCESS_EXECUTOR_IMPL_HPP
#define PROCESS_EXECUTOR_IMPL_HPP

#include "executor.hpp"

namespace process {
    namespace details {

        template <typename... Args>
        inline child execute_impl(Args &&...args) {
            auto exec = make_executor(std::forward<Args>(args)...);
            return exec();
        }

    } // namespace details
} // namespace process

#endif // PROCESS_EXECUTOR_IMPL_HPP
