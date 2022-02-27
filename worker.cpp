#include "worker.h"

#include <iostream>
#include <functional>

#include <boost/system/error_code.hpp>
#include <boost/optional.hpp>
#include "service.h"
#include "node_p_diagnostic.h"

Worker::Worker(boost::asio::io_service &service, std::function< void( Worker* ) > accept_handler, LLogger& logger ) :
    socket_( service ), accept_handler_( accept_handler ), logger_(logger)
{ }

void Worker::accept( const Worker::ErrorCode &error_code )
{
    logger_.Log(LLogger::Level::Medium, "New connection | tid: $$", std::this_thread::get_id());
    if ( !error_code ) {
        read_request();
    }
}

void Worker::read_request()
{
    socket_.async_read_some( boost::asio::buffer( read_buffer_.data(), read_buffer_.size() ),
                             [ this ] ( Worker::ErrorCode error_code, std::size_t bytes_transferred ) {
        if ( !error_code ) {
            std::string result;
            try {
                read_buffer_[bytes_transferred] = '\0';
                result = service::execute( read_buffer_.data(), logger_ );

            }
            catch(const std::exception& ex) {
                result = lformat::format( "{\"error\": \"$$\"}", ex.what() );
            }
            std::copy(result.begin(), result.end(), write_buffer_.begin());
            logger_.Log(LLogger::Level::Medium, "Thread $$ finish execute", std::this_thread::get_id());
            logger_.Log(LLogger::Level::Medium, "Not released $$ nodes", AllacationCount_Node);
            write_buffer_[result.size()] = '\0';
            socket_.async_write_some( boost::asio::buffer( write_buffer_.data(), result.size() ),
                                      [this] ( Worker::ErrorCode, std::size_t )
            {
                read_request();
            });
        }
        else {
            accept_handler_( this );
        }
    });
}
