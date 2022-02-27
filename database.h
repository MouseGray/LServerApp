#pragma once

#include <optional>

#include <pqxx/pqxx>

struct DataBaseException : public std::runtime_error {
    using Base = std::runtime_error;
    DataBaseException( const std::string& error ) : Base( error )
    { }
};

class DataBase
{
public:
    DataBase();
    bool ConnectToStaticDB( std::string host,
                            std::string port,
                            std::string user,
                            std::string pass,
                            std::string dbname ) noexcept;
    bool ConnectToApplicationDB( std::string host,
                                 std::string port,
                                 std::string user,
                                 std::string pass,
                                 std::string dbname ) noexcept;
    pqxx::connection& Get( const std::string& db ) ;
private:
    std::string _static_host = "localhost";
    std::string _static_port = "5432";
    std::string _static_user = "postgres";
    std::string _static_pass = "postgres";
    std::string _static_dbname = "static";
    std::optional< pqxx::connection > _static_db;
    std::string _application_host = "localhost";
    std::string _application_port = "5432";
    std::string _application_user = "postgres";
    std::string _application_pass = "postgres";
    std::string _application_dbname = "application";
    std::optional< pqxx::connection > _application_db;
};

class DataBaseAcceptor
{
public:
    static DataBase& Database();
private:
    static void Init();
    static DataBase* _database;
};
