#include "database.h"

#include <iostream>
#include <format>

DataBase::DataBase()
{

}

bool DataBase::ConnectToStaticDB( std::string host,
                                  std::string port,
                                  std::string user,
                                  std::string pass,
                                  std::string dbname ) noexcept {
//    _static_host = std::move( host );
//    _static_port = std::move( port );
//    _static_user = std::move( user );
//    _static_pass = std::move( pass );
//    _static_dbname = std::move( dbname );
    try {
        _static_db = pqxx::connection(
                    //std::format( "dbname = {} user = {} password = {} hostaddr = {} port = {}",
                    //             dbname, user, pass, host, port ) );
                    "dbname=" + dbname +
                    " user=" + user +
                    " password=" + pass +
                    " host=" + host +
                    " port=" + port);
    }
    catch ( const pqxx::broken_connection& ex ) {
        std::cout << ex.what();
        return false;
    }
    catch ( const std::exception& ex ) {
        std::cout << ex.what();
        return false;
    }
    return true;
}

bool DataBase::ConnectToApplicationDB( std::string host,
                                       std::string port,
                                       std::string user,
                                       std::string pass,
                                       std::string dbname ) noexcept {
//    _application_host = std::move( host );
//    _application_port = std::move( port );
//    _application_user = std::move( user );
//    _application_pass = std::move( pass );
//    _application_dbname = std::move( dbname );
    try {
        _application_db = pqxx::connection(
                    "dbname=" + dbname +
                    " user=" + user +
                    " password=" + pass +
                    " host=" + host +
                    " port=" + port);
    }
    catch ( const pqxx::broken_connection& ex ) {
        std::cout << ex.what();
        return false;
    }
    catch ( const std::exception& ex ) {
        std::cout << ex.what();
        return false;
    }
    return true;
}

pqxx::connection &DataBase::Get( const std::string &db )
{
    if ( db == "Static" ) {
        if ( !_static_db.has_value() ) {
            if ( !ConnectToStaticDB( _static_host,
                                     _static_port,
                                     _static_user,
                                     _static_pass,
                                     _static_dbname ) ) {
                throw DataBaseException( "Не удалось установить соединение с БД [Static]" );
            }
        }
        return _static_db.value();
    }
    else if ( db == "Application" ) {
        if ( !_application_db.has_value() ) {
            if ( !ConnectToApplicationDB( _application_host,
                                          _application_port,
                                          _application_user,
                                          _application_pass,
                                          _application_dbname ) ) {
                throw DataBaseException( "Не удалось установить соединение с БД [Application]" );
            }
        }
        return _application_db.value();
    }
    throw std::invalid_argument("Undefined DataBase");
}

DataBase* DataBaseAcceptor::_database = nullptr;

DataBase& DataBaseAcceptor::Database()
{
    if ( DataBaseAcceptor::_database == nullptr ) {
        DataBaseAcceptor::Init();
    }
    return *DataBaseAcceptor::_database;
}

void DataBaseAcceptor::Init()
{
    DataBaseAcceptor::_database = new DataBase();
}
