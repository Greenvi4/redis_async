//
// Created by niko on 24.05.2021.
//

#include <redis_async/error.hpp>

namespace redis_async {
    namespace error {

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
