//
// Created by niko on 24.05.2021.
//

#include <redis_async/error.hpp>

namespace redis_async {
    namespace error {

        const char *category::name() const noexcept {
            return "redis_async";
        }

        std::string category::message(int ev) const {
            auto ec = static_cast<errc>(ev);
            switch (ec) {
            case errc::wrong_introduction:
                return "Wrong introduction";
            case errc::parser_error:
                return "Parser error";
            case errc::count_conversion:
                return "Cannot convert count to number";
            case errc::count_range:
                return "Unacceptable count value";
            case errc::bulk_terminator:
                return "Terminator for bulk string not found";
            case errc::not_enough_data:
                return "Buffer just does not contain enough information to completely parse it";
            }
            return "Unknown protocol error";
        }

        std::error_code make_error_code(errc e) {
            return {static_cast<int>(e), get_error_category()};
        }

        rd_error::rd_error(const std::string &msg)
            : runtime_error(msg) {
        }
        rd_error::rd_error(const char *msg)
            : runtime_error(msg) {
        }
        connection_error::connection_error(const std::string &msg)
            : rd_error(msg) {
        }
        connection_error::connection_error(const char *msg)
            : rd_error(msg) {
        }
        query_error::query_error(const std::string &msg)
            : rd_error(msg) {
        }
        query_error::query_error(const char *msg)
            : rd_error(msg) {
        }
        client_error::client_error(const std::string &msg)
            : rd_error(msg) {
        }
        client_error::client_error(const char *msg)
            : rd_error(msg) {
        }
        client_error::client_error(const std::exception &e)
            : rd_error(std::string("Client thrown exception: ") + e.what()) {
        }

    } // namespace error
} // namespace redis_async
