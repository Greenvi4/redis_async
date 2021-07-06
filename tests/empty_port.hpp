//
// Created by niko on 11.06.2021.
//

#ifndef REDIS_ASYNC_EMPTY_PORT_HPP
#define REDIS_ASYNC_EMPTY_PORT_HPP

#include <arpa/inet.h>
#include <netdb.h> /* Needed for getaddrinfo() and freeaddrinfo() */
#include <sys/socket.h>
#include <unistd.h> /* Needed for close() */

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <random>
#include <stdexcept>
#include <thread>

namespace empty_port {

    using socket_t = int;
    using port_t = uint16_t;
    using clock_t = std::chrono::high_resolution_clock;

    class SocketHolder {
        socket_t socket_;

    public:
        SocketHolder(socket_t s)
            : socket_(s) {
            if (s == -1) {
                throw std::runtime_error("Cannot get socket");
            }
        }

        ~SocketHolder() {
            close(socket_);
        }

        SocketHolder(const SocketHolder &) = delete;
        SocketHolder& operator=(const SocketHolder &) = delete;

        socket_t &socket() {
            return socket_;
        }
    };

    static constexpr port_t MIN_PORT = 49152;
    static constexpr port_t MAX_PORT = 65535;

    struct impl {
        static bool can_listen(port_t port, const char *host);
        static void fill_struct(empty_port::socket_t socket, sockaddr_in &addr, port_t port,
                                const char *host);
        static bool check_port_impl(port_t port, const char *host);
        static port_t get_random_impl(const char *host);
        template <typename D>
        static bool wait_port_impl(port_t port, const char *host, D max_wait);
    };

    /* TCP impl */
    template <typename D>
    inline bool impl::wait_port_impl(port_t port, const char *host, D max_wait) {
        auto stop_at = clock_t::now() + max_wait;
        do {
            if (!impl::check_port_impl(port, host)) {
                return true;
            }
            std::this_thread::sleep_for(D(1));
        } while (clock_t::now() < stop_at);

        return false;
    }

    inline port_t impl::get_random_impl(const char *host) {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<port_t> uni(MIN_PORT, MAX_PORT);

        port_t some_port = uni(rng);
        while (some_port < MAX_PORT) {
            bool is_empty = impl::check_port_impl(some_port, host) &&
                            impl::can_listen(some_port, host);
            if (is_empty) {
                return some_port;
            }
            some_port++;
        }
        throw std::runtime_error("Cannot get random port");
    }

    inline void impl::fill_struct(empty_port::socket_t  /*socket*/, sockaddr_in &addr, const port_t port,
                                  const char *host) {
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        addrinfo hints, *result = nullptr;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        struct sockaddr_in *host_addr;
        // looks up IPv4/IPv6 address by host name or stringized IP address
        int r = getaddrinfo(host, nullptr, &hints, &result);
        if (r) {
            throw std::runtime_error(std::string("Cannot getaddrinfo:: ") + strerror(r));
        }
        host_addr = (struct sockaddr_in *)result->ai_addr;
        memcpy(&addr.sin_addr, &host_addr->sin_addr, sizeof(struct in_addr));
        freeaddrinfo(result);
    }

    inline bool impl::can_listen(port_t port, const char *host) {
        SocketHolder sh(socket(AF_INET, SOCK_STREAM, 0));
        sockaddr_in addr;
        fill_struct(sh.socket(), addr, port, host);

        if (bind(sh.socket(), (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            return false;
        }
        if (listen(sh.socket(), 1)) {
            return false;
        }

        /* success */
        return true;
    }

    inline bool impl::check_port_impl(port_t port, const char *host) {
        SocketHolder sh(socket(AF_INET, SOCK_STREAM, 0));
        sockaddr_in remote_addr;

        fill_struct(sh.socket(), remote_addr, port, host);

        if ((connect(sh.socket(), (struct sockaddr *)&remote_addr, sizeof(remote_addr)))) {
            return true;
        } else {
            return false;
        }
    }

    /* public interface */
    inline bool check_port(port_t port, const char *host = "127.0.0.1") {
        return impl::check_port_impl(port, host);
    }

    inline port_t get_random(const char *host = "127.0.0.1") {
        return impl::get_random_impl(host);
    }

    template <typename D = std::chrono::milliseconds>
    inline bool wait_port(const port_t port, const char *host = "127.0.0.1", D max_wait = D(500)) {
        return impl::template wait_port_impl<D>(port, host, max_wait);
    }
} // namespace empty_port

#endif // REDIS_ASYNC_EMPTY_PORT_HPP
