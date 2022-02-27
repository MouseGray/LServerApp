#pragma once

#include <array>
#include <optional>
#include <functional>

#include <boost/asio.hpp>

#include <lloggerlib.h>

class Worker
{
    using ErrorCode = boost::system::error_code;

    using Net = boost::asio::ip::tcp;

    static constexpr std::size_t buffer_size = 8196;
public:
    Worker( boost::asio::io_service& service, std::function<void( Worker* )> accept_handler, LLogger& logger );

    Worker( const Worker& ) = delete;

    inline boost::asio::ip::tcp::socket& socket() noexcept { return socket_; }

    void accept( const ErrorCode &error_code );
private:
    void handle_connection( const ErrorCode& error_code );

    void read_request();

    boost::asio::ip::tcp::socket socket_;

    std::function<void( Worker* )> accept_handler_;

    std::array< char, buffer_size > read_buffer_;
    std::array< char, buffer_size > write_buffer_;

    LLogger& logger_;
};
