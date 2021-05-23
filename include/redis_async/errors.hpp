//
// Created by niko on 23.05.2021.
//

#ifndef REDIS_ASYNC_ERRORS_HPP
#define REDIS_ASYNC_ERRORS_HPP

#include <system_error>

namespace redis_async {

    enum class redis_error { invalid_connection_opt = 1 };

    class redis_category_impl : public std::error_category {
    public:
        const char *name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline const std::error_category &redis_category() {
        static redis_category_impl instance;
        return instance;
    }

    inline std::error_code make_error_code(redis_error e) {
        return std::error_code(static_cast<int>(e), redis_category());
    }

    inline std::error_condition make_error_condition(redis_error e) {
        return std::error_condition(static_cast<int>(e), redis_category());
    }

    inline const char *redis_category_impl::name() const noexcept {
        return "redis";
    }

    inline std::string redis_category_impl::message(int ev) const {
        switch (static_cast<redis_error>(ev)) {
        case redis_error::invalid_connection_opt:
            return "invalid connection string";
        }
        return "Unknown Redis error";
    }

} // namespace redis_async

namespace std {
    template <>
    struct is_error_code_enum<redis_async::redis_error> : public true_type {};
} // namespace std

#endif // REDIS_ASYNC_ERRORS_HPP
