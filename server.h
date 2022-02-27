#pragma once

#include <list>

#include <boost/asio.hpp>

#include <lloggerlib.h>

#include "worker.h"

class Server
{
    using Net = boost::asio::ip::tcp;
    using Endpoint = Net::endpoint;

    const Net IPv4 = Net::v4();
public:
    Server( const int port, LLogger &logger );

    void exec( const int worker_count, const int thread_count );

    void join();
private:
    void accept( Worker* worker );

    void add_worker();

    void remove_worker( Worker *worker );

    void thread_run();

    std::mutex mutex_;

    boost::asio::io_service io_service_;
    Net::acceptor acceptor_;

    std::list< Worker* > workers_;
    std::vector<std::thread> threads_;

    LLogger& logger_;
};
