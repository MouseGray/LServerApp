#include "service.h"

#include <regex>

#include <pqxx/pqxx>

#include <rapidjson/error/en.h>
#include <ranges>

#include "database.h"
#include "node_p_diagnostic.h"

#include "utils.h"

std::string service::execute( const char *data, LLogger& logger )
{
    logger.Log(LLogger::Level::Medium, "Thread $$ start execute", std::this_thread::get_id());
    logger.Log(LLogger::Level::Medium, "Not released $$ nodes", AllacationCount_Node);

    rapidjson::Document json_data;
    rapidjson::ParseResult ok = json_data.Parse( data );

    if (!ok) {
        throw logger.Exception("JSON parse error: $$ ($$)",
                               rapidjson::GetParseError_En(ok.Code()), ok.Offset());
    }

    const auto fields = std::make_tuple(jsonutils::JsonNameType<jsonutils::Type::Number>{ "task" },
                                        jsonutils::JsonNameType<jsonutils::Type::String>{ "uuid" });


    auto error = jsonutils::has_fields(json_data, fields);
    if (!error.is_null()) {
        throw logger.Exception("Not found json-field {$$ : $$}.", error.name, error.type);
    }

    const std::string uuid = json_data["uuid"].GetString();
    const int taskid = json_data["task"].GetInt();

    const std::string text = getTaskUserText( uuid, taskid );

    auto addition_data = getTaskStaticData( taskid );

    LTask task( text.c_str(), logger );
    task.SetVariables( std::get< 0 >( addition_data ) );
    task.SetAdditionData( std::get< 1 >( addition_data ),
                          std::get< 2 >( addition_data ),
                          std::get< 3 >( addition_data ) );
    task.Deploy();
    task.CheckAll();

    return task.Result();
}

std::string service::getTaskUserText( const std::string& uuid, int taskid )
{
    pqxx::nontransaction nt( DataBaseAcceptor::Database().Get( "Application" ) );
    pqxx::result result = nt.exec_params( "SELECT solution "
                                          "FROM user_task "
                                          "WHERE user_uuid = $1 AND task_id = $2", uuid, taskid );
    if ( result.empty() ) return "";

    return result[ 0 ].at( "solution" ).c_str();
}

std::tuple<std::vector<char>, std::string, std::string, std::string>
service::getTaskStaticData( int taskid )
{
    std::tuple<std::vector<char>, std::string, std::string, std::string>
            result_tuple;
    pqxx::nontransaction nt( DataBaseAcceptor::Database().Get( "Static" ) );
    pqxx::result result = nt.exec_params( "SELECT variables, given, unknown, answer "
                                          "FROM task "
                                          "WHERE taskid = $1", taskid );
    if ( result.empty() ) return result_tuple;
    if (!result[ 0 ].at( "variables" ).is_null()) {
        std::string variables = result[ 0 ].at( "variables" ).as<std::string>();
        std::get< 0 >( result_tuple ) = utils::first_char(
                    utils::split(variables .substr(1, variables.size() - 2), ','));
    }
    if (!result[ 0 ].at( "given" ).is_null())
        std::get< 1 >( result_tuple ) = result[ 0 ].at( "given" ).c_str();
    if (!result[ 0 ].at( "unknown" ).is_null())
        std::get< 2 >( result_tuple ) = result[ 0 ].at( "unknown" ).c_str();
    if (!result[ 0 ].at( "answer" ).is_null())
        std::get< 3 >( result_tuple ) = result[ 0 ].at( "answer" ).c_str();
    return result_tuple;
}

