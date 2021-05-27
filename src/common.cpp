//
// Created by niko on 23.05.2021.
//

#include <redis_async/common.hpp>
#include <redis_async/error.hpp>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>

#include <tuple>
#include <vector>

namespace redis_async {

    struct connect_string_parser {

        connect_string_parser() = default;

        connection_options operator()(std::string const &uri) {
            connection_options opts;
            std::string auth;
            std::string path;
            std::tie(auth, path) = split_uri(uri, opts);

            set_auth_opts(auth, opts);
            auto parameter_string = split_path(path, opts);
            parse_parameters(parameter_string, opts);

            return opts;
        }

        static auto split_uri(const std::string &uri, connection_options &opts)
            -> std::tuple<std::string, std::string>;
        static void set_auth_opts(const std::string &auth, connection_options &opts);
        static auto split_path(const std::string &path, connection_options &opts) -> std::string;
        static void parse_parameters(const std::string &parameter_string, connection_options &opts);
        static void set_option(const std::string &key, const std::string &val,
                               connection_options &opts);
        static std::chrono::milliseconds _parse_timeout_option(const std::string &str);
        static bool parse_bool_option(const std::string &str);
    };

    auto connect_string_parser::split_uri(const std::string &uri, connection_options &opts)
        -> std::tuple<std::string, std::string> {
        auto pos = uri.find("=");
        if (pos == std::string::npos)
            throw error::connection_error("invalid connection string");

        opts.alias = uri.substr(0, pos);
        if (opts.alias.empty())
            throw error::connection_error("invalid connection string");

        auto start = pos + 1;

        pos = uri.find("://", start);
        if (pos == std::string::npos)
            throw error::connection_error("invalid connection string");

        opts.schema = uri.substr(start, pos - start);
        if (opts.schema.empty())
            throw error::connection_error("invalid connection string");

        start = pos + 3;
        pos = uri.find("@", start);
        if (pos == std::string::npos) {
            // No auth info.
            return std::make_tuple(std::string{}, uri.substr(start));
        }

        auto auth = uri.substr(start, pos - start);

        return std::make_tuple(auth, uri.substr(pos + 1));
    }

    void connect_string_parser::set_auth_opts(const std::string &auth, connection_options &opts) {
        if (auth.empty()) {
            // No auth info.
            return;
        }

        auto pos = auth.find(":");
        if (pos == std::string::npos) {
            // No user name.
            opts.password = auth;
        } else {
            opts.user = auth.substr(0, pos);
            opts.password = auth.substr(pos + 1);
        }
    }
    auto connect_string_parser::split_path(const std::string &path, connection_options &opts)
        -> std::string {
        auto parameter_pos = path.rfind("?");
        std::string parameter_string;
        if (parameter_pos != std::string::npos) {
            parameter_string = path.substr(parameter_pos + 1);
        }

        auto pos = path.rfind("/");
        if (pos != std::string::npos) {
            // Might specified a db number.
            try {
                auto db = path.substr(pos + 1);
                std::stoi(db);
                opts.database = std::move(db);
                opts.uri = path.substr(0, pos);
            } catch (const std::exception &) {
                // Not a db number, and it might be a path to unix domain socket.
                opts.uri = path.substr(0, parameter_pos);
            }
        } else {
            opts.uri = path.substr(0, parameter_pos);
        }

        if (opts.uri.empty())
            throw error::connection_error("invalid connection string");

        // No db number specified, and use default one, i.e. 0.
        return parameter_string;
    }
    void connect_string_parser::parse_parameters(const std::string &parameter_string,
                                                 connection_options &opts) {
        if (parameter_string.empty())
            return;
        using Strings = std::vector<std::string>;

        Strings parameters;
        boost::split(parameters, parameter_string, boost::is_any_of("&"));

        for (const auto &parameter : parameters) {
            Strings kv_pair;
            boost::split(kv_pair, parameter, boost::is_any_of("="));
            if (kv_pair.size() != 2) {
                throw error::connection_error("invalid option: not a key-value pair: " + parameter);
            }

            const auto &key = kv_pair[0];
            const auto &val = kv_pair[1];
            set_option(key, val, opts);
        }
    }
    void connect_string_parser::set_option(const std::string &key, const std::string &val,
                                           connection_options &opts) {
        if (key == "keep_alive") {
            opts.keep_alive = parse_bool_option(val);
        } else if (key == "connect_timeout") {
            opts.connect_timeout = _parse_timeout_option(val);
        } else if (key == "socket_timeout") {
            opts.socket_timeout = _parse_timeout_option(val);
        } else {
            throw error::connection_error("unknown uri parameter " + key);
        }
    }
    std::chrono::milliseconds connect_string_parser::_parse_timeout_option(const std::string &str) {
        std::size_t timeout = 0;
        std::string unit;
        try {
            std::size_t pos = 0;
            timeout = std::stoul(str, &pos);
            unit = str.substr(pos);
        } catch (const std::exception &e) {
            throw error::connection_error("invalid uri parameter of timeout type: " + str);
        }
        if (unit == "ms") {
            return std::chrono::milliseconds(timeout);
        } else if (unit == "s") {
            return std::chrono::seconds(timeout);
        } else if (unit == "m") {
            return std::chrono::minutes(timeout);
        } else {
            throw error::connection_error("unknown timeout unit: " + unit);
        }
    }
    bool connect_string_parser::parse_bool_option(const std::string &str) {
        auto value = boost::to_lower_copy(str);
        if (value == "true") {
            return true;
        } else if (value == "false") {
            return false;
        }
        throw error::connection_error("invalid uri parameter of bool type: " + str);
    }

    connection_options connection_options::parse(const std::string &uri) {
        return connect_string_parser()(uri);
    }
} // namespace redis_async

redis_async::rdalias operator""_rd(const char *v, size_t) {
    return redis_async::rdalias{std::string{v}};
}
redis_async::connection_options operator""_redis(const char *uri, size_t) {
    return redis_async::connection_options::parse(uri);
}
