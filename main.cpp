#include <iostream>

#include <boost/system/error_code.hpp>

#include <pqxx/pqxx>

#include <boost/optional/optional.hpp>

#include <lloggerlib.h>
#include <lconfig.h>

#include "server.h"

int main( int /*argc*/, char** /*argv*/ )
{
    LLogger logger(LLogger::Level::High, "Log.txt");
    // ===== From config ======
    LConfig config(logger);
    config.Init();

    if (!config.Has("port")) {
        logger.Error(LLogger::Level::Low, "Option 'port' is not defined in config file!");
        return 1;
    }
    const int port = std::stoi(config.Get("port"));
    // ========================

    try {
        Server server(port, logger);
        server.exec(3, 3);
        server.join();
    }
    catch( const boost::system::system_error& ) {
        // TODO: Log
    }

    return 0;
}
