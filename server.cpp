#include "server.h"

#include <boost/bind.hpp>

Server::Server( const int port , LLogger &logger ) :
    acceptor_( io_service_, Endpoint( IPv4, port ) ), logger_(logger)
{ }

void Server::exec( const int worker_count, const int thread_count )
{
    std::vector< boost::asio::ip::tcp::socket > sockets;
    for ( auto i = 0; i < worker_count; ++i )
    {
        add_worker();
        acceptor_.async_accept( workers_.back()->socket(),
                                std::bind( &Worker::accept, workers_.back(), std::placeholders::_1 ) );
    }
    for ( auto i = 0; i < thread_count; ++i )
    {
        threads_.emplace_back( std::thread( &Server::thread_run, this ) );
    }
    //io_service_.run();
}

void Server::join()
{
    for ( auto& thread : threads_ )
    {
        thread.join();
    }
}

void Server::accept( Worker *worker )
{
    remove_worker( worker );
    add_worker();
    acceptor_.async_accept( workers_.back()->socket(),
                            std::bind( &Worker::accept, workers_.back(), std::placeholders::_1 ) );
}

void Server::add_worker()
{
    std::lock_guard lock( mutex_ );
    workers_.push_back( new Worker{ io_service_, std::bind( &Server::accept, this, std::placeholders::_1 ), logger_ } );
}

void Server::remove_worker( Worker* worker )
{
    auto worker_it = std::find( workers_.begin(), workers_.end(), worker );
    std::lock_guard lock( mutex_ );
    delete *worker_it;
    workers_.erase( worker_it );
}

void Server::thread_run()
{
    logger_.Log(LLogger::Level::Medium, "Thread $$ started", std::this_thread::get_id());
    io_service_.run();
    logger_.Log(LLogger::Level::Medium, "Thread $$ finished", std::this_thread::get_id());
}

