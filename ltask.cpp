#include "ltask.h"

#include <format>

#include <lexpression2.h>
#include <lvalidator.h>
#include <lcheck.h>

#include <lvector.h>

#include <rapidjson/error/en.h>

LTask::LTask( const char *data, LLogger& logger )
    : mLogger(logger)
{
    rapidjson::Document json_data;
    rapidjson::ParseResult ok = json_data.Parse( data );

    if (!ok) {
        mLogger.Warning(LLogger::Level::Low, "JSON parse error: $$ ($$)",
                        rapidjson::GetParseError_En(ok.Code()), ok.Offset());
        return;
    }

    if (!json_data.IsArray()) {
        mLogger.Warning(LLogger::Level::Low, "Task json is not {Array}");
        return;
    }

    auto json_array = json_data.GetArray();

    const auto fields = std::make_tuple(jsonutils::JsonNameType<jsonutils::Type::String>{ "left" },
                                        jsonutils::JsonNameType<jsonutils::Type::String>{ "right" },
                                        jsonutils::JsonNameType<jsonutils::Type::Number>{ "operation" },
                                        jsonutils::JsonNameType<jsonutils::Type::Number>{ "uid" },
                                        jsonutils::JsonNameType<jsonutils::Type::Array>{ "markers" },
                                        jsonutils::JsonNameType<jsonutils::Type::Array>{ "numbers" });

    const auto markers_fields = std::make_tuple(jsonutils::JsonNameType<jsonutils::Type::Array>{ "left" },
                                                jsonutils::JsonNameType<jsonutils::Type::Array>{ "right" });

    for (auto& json_element: json_array) {
        LRow row;

        auto error = jsonutils::has_fields(json_element, fields);
        if (!error.is_null()) {
            mLogger.Warning(LLogger::Level::Medium, "Not found json-field {$$ : $$}.",
                            error.name, error.type);
            continue;
        }

        row.left        = json_element["left"].GetString();
        row.right       = json_element["right"].GetString();
        row.operation   = json_element["operation"].GetInt();
        row.uid         = json_element["uid"].GetInt();
        row.connection  = 0;
        row.error = lexpr::Error::NoCheck;
        row.error_text = "Не проверено";

        auto markers = json_element["markers"].GetArray();

        for (rapidjson::SizeType idx = 0; idx < markers.Size(); ++idx) {
            auto error = jsonutils::has_fields(markers[idx], markers_fields);
            if (!error.is_null()) {
                mLogger.Warning(LLogger::Level::Medium, "Not found json-field {$$ : $$}.",
                                error.name, error.type);
                continue;
            }

            auto left = markers[idx]["left"].GetArray();
            for (auto& marker: left) {
                if (!marker.IsInt()) {
                    mLogger.Warning(LLogger::Level::Medium, "Array {markers.left} element in not {int}.");
                    continue;
                }
                row.markers[idx].left_marked.push_back(marker.GetInt());
            }
            auto right = markers[idx]["right"].GetArray();
            for (auto& marker: right) {
                if (!marker.IsInt()) {
                    mLogger.Warning(LLogger::Level::Medium, "Array {markers.right} element in not {int}.");
                    continue;
                }
                row.markers[idx].right_marked.push_back(marker.GetInt());
            }
        }

        auto numbers = json_element["numbers"].GetArray();
        for (rapidjson::SizeType idx = 0; idx < numbers.Size(); ++idx) {
            if (!numbers[idx].IsInt()) {
                mLogger.Warning(LLogger::Level::Medium, "Array {numbers} element in not {int}.");
                row.numbers[ idx ] = -1;
                continue;
            }
            row.numbers[ idx ] = numbers[idx].GetInt();
        }
        mRows.push_back(std::move(row));
    }
}

void LTask::SetVariables(const std::vector<char> &variables)
{
    lexpr::lvector_generator<lexpr::symbol_t> generator;
    const std::int32_t size = static_cast<std::int32_t>(variables.size());
    const std::int32_t system_size = 1 << size;
    for ( std::int32_t it = 0; it < size; ++it ) {
        mBaseValues.insert( { variables[ it ], generator(system_size, 1 << it, {0, 1}) } );
        mLogger.Log(LLogger::Level::High, "Generated '$$': $$.", variables[ it ],
                    generator(system_size, 1 << it, {0, 1}).to_string());
    }

    mBaseValues.insert( { lexpr::specSymbol::Nullset, generator(system_size, 0, {0} ) } );
    mLogger.Log(LLogger::Level::High, "Generated 'N': $$.",
                generator(system_size, 0, {0}).to_string());

    mBaseValues.insert( { lexpr::specSymbol::Uniset, generator(system_size, 0, {1} ) } );
    mLogger.Log(LLogger::Level::High, "Generated 'U': $$.",
                generator(system_size, 0, {1}).to_string());
}

void LTask::SetAdditionData(const std::string &given, const std::string &unknown, const std::string &answer)
{
    fillExtraData(given.c_str(), mGivens);
    fillExtraData(unknown.c_str(), mUnknowns);
    fillExtraData(answer.c_str(), mAnswer);
}

void LTask::fillExtraData(const char* json, std::vector<ExtraData> &extra_data)
{
    rapidjson::Document json_data;
    rapidjson::ParseResult ok = json_data.Parse(json);

    if (!ok) {
        mLogger.Warning(LLogger::Level::Low, "JSON parse error: $$ ($$)",
                        rapidjson::GetParseError_En(ok.Code()), ok.Offset());
        return;
    }

    if (!json_data.IsArray()) {
        mLogger.Warning(LLogger::Level::Low, "Task json is not {Array}");
        return;
    }

    auto json_array = json_data.GetArray();

    const auto fields = std::make_tuple(jsonutils::JsonNameType<jsonutils::Type::String>{ "left" },
                                        jsonutils::JsonNameType<jsonutils::Type::String>{ "right" });

    for (auto& json_element: json_array) {
        auto error = jsonutils::has_fields(json_element, fields);
        if (!error.is_null()) {
            mLogger.Warning(LLogger::Level::Medium, "Not found json-field {$$ : $$}.",
                            error.name, error.type);
            continue;
        }
        extra_data.push_back( { json_element["left"].GetString(), json_element["right"].GetString() } );
    }
}

void LTask::Deploy()
{
    for( auto& row: mRows ) {
        auto lres = validator::is_valid_expression( row.left );
        if ( lres.second != validator::ErrorCode::no_error ) {
            row.error = lexpr::Error::SyntaxError;
            row.error_text = validator::error_text(lres, true);
            continue;
        }
        auto rres = validator::is_valid_expression( row.right );
        if ( rres.second != validator::ErrorCode::no_error ) {
            row.error = lexpr::Error::SyntaxError;
            row.error_text = validator::error_text(rres, false);
            continue;
        }
        row.ltree.reset(lexpr::create2( row.left ));
        row.rtree.reset(lexpr::create2( row.right ));

        row.lextree.reset( row.ltree->copy() );
        lexpr::remove_brackets( row.lextree.get() );
        lexpr::convert_to_multioperand_expr( row.lextree.get() );

        row.rextree.reset(row.rtree->copy());
        lexpr::remove_brackets( row.rextree.get() );
        lexpr::convert_to_multioperand_expr( row.rextree.get() );
    }

    deployExtraData(mGivens);
    deployExtraData(mUnknowns);
    deployExtraData(mAnswer);
}

void LTask::deployExtraData(std::vector<ExtraData> &extra_data)
{
    for( auto& extra_element: extra_data ) {
        auto lres = validator::is_valid_expression( extra_element.left );
        if ( lres.second != validator::ErrorCode::no_error ) {
            continue;
        }
        auto rres = validator::is_valid_expression( extra_element.right );
        if ( rres.second != validator::ErrorCode::no_error ) {
            continue;
        }
        extra_element.ltree.reset( lexpr::create2( extra_element.left ) );
        extra_element.rtree.reset( lexpr::create2( extra_element.right ) );
    }
}

bool is_operator(char val) {
    switch(val){
    case '#':
    case '$':
    case '^':
    case '-':
    case '+':
    case '*':
    case '/':
    case '{':
    case '}':
    case '|':
    case '(':
    case ')':
    case '`':
    case '=':
    case '.':
        return true;
    }
    return false;
}

std::pair<bool, char> LTask::validationData(const LRow &row)
{
    for( auto val : row.left ){
        if (!mBaseValues.count(val) && !is_operator(val) && !isdigit(val) && val != 'X')
            return { true, val };
    }
    for( auto val : row.right ){
        if (!mBaseValues.count(val) && !is_operator(val) && !isdigit(val) && val != 'X')
            return { true, val };
    }
    return { false, '\0' };
}

LRow& LTask::find(int uid) {
    for ( auto& row: mRows ) {
        if (row.uid == uid) return row;
    }
    return mRows.front();
}

void LTask::CheckAll()
{
    for ( auto& row: mRows ) {
        if ( row.error != lexpr::Error::NoCheck ) continue;
        auto error = validationData(row);
        if ( error.first ) {
            row.error = lexpr::Error::SyntaxError;
            row.error_text = lformat::format("Не известный символ '$$'", error.second);
        }
    }

    int conn = 0;
    mLogger.Log(LLogger::Level::Medium, "Not released $$ nodes", AllacationCount_Node);
    for ( auto& row: mRows ) {
        if ( row.error != lexpr::Error::NoCheck ) continue;

        // Проверка на равенство
        if ( lcheck::is_need_check_equality(row.operation) ) {
            try {
                if (lexpr::calculate( row.ltree.get(), mBaseValues)
                     != lexpr::calculate( row.rtree.get(), mBaseValues)) {
                    row.error = lexpr::Error::EqualityError;
                    row.error_text = "Выражения не равны";
                    continue;
                }
            }  catch (std::invalid_argument& ex) {
                row.error = lexpr::Error::UnknownError;
                row.error_text = ex.what();
                continue;
            }
            catch (std::bad_alloc& ex) {
                row.error = lexpr::Error::UnknownError;
                row.error_text = ex.what();
                continue;
            }
        }

        // Проверка на правильность маркеров
        for ( std::size_t i = 0; i < row.markers.size(); ++i ){
            if ( !lexpr::is_valid_numbers( row.ltree.get(), row.markers[ i ].left_marked ) ||
                 !lexpr::is_valid_numbers( row.rtree.get(), row.markers[ i ].right_marked )) {
                row.error = lexpr::Error::OperationError;
                row.error_text = "Некорректно указаны маркеры";
                break;
            }
        }

        if ( row.error != lexpr::Error::NoCheck ) continue;

        // Проверка операции
        if ( row.operation != lcheck::change_equal  &&
             row.operation != lcheck::substitution  &&
             row.operation != lcheck::conclusion    &&
             row.operation != lcheck::given         &&
             row.operation != lcheck::unknown       &&
             row.operation != lcheck::answer ) {
            if ( !lcheck::check_expression( static_cast<lcheck::operations> (row.operation ),
                                            row.ltree.get(), row.rtree.get(), row.markers ) ) {
                row.error = lexpr::Error::OperationError;
                row.error_text = "Некорректно указана операция или она не элементарна";
                continue;
            }
            else {
                row.connection = findConnection(row.uid, row.lextree.get(), conn);
                row.error = lexpr::Error::NoError;
                row.error_text = "Ошибок нет";
                continue;
            }
        }

        // Проверка остальных операций

        if ( row.operation == lcheck::change_equal ) {
            if ( row.numbers[0] == -1 ) continue;
            if ( row.numbers[0] < 0 || static_cast<int>(mRows.size()) <= row.numbers[0] ) {
                row.error = lexpr::Error::UnknownError;
                row.error_text = "Строки с указанным номером не существует";
                continue;
            }
            if ( !lcheck::check_change_equal( find( row.numbers[0] ).ltree.get(),
                                              find( row.numbers[0] ).rtree.get(),
                                              row.ltree.get(), row.rtree.get() ) ) {
                row.error = lexpr::Error::OperationError;
                row.error_text = "Некорректно указана операция или она не элементарна";
                continue;
            }
            else {
                row.connection = findConnection(row.uid, row.lextree.get(), conn);
                row.error = lexpr::Error::NoError;
                row.error_text = "Ошибок нет";
                continue;
            }
        }
        if ( row.operation == lcheck::substitution ) {
            std::vector<std::pair<lexpr::node_e*,
                    lexpr::node_e*>> dat;
            for ( auto i: row.numbers ) {
                if ( i == -1 )
                    dat.push_back({nullptr, nullptr});
                else
                    dat.push_back({find( i ).ltree.get(), find( i ).rtree.get()});
            }
            if ( !lcheck::check_substitution( dat,
                                              row.ltree.get(), row.rtree.get(), row.markers ) ) {
                row.error = lexpr::Error::OperationError;
                row.error_text = "Некорректно указана операция или она не элементарна";
                continue;
            }
            else {
                row.connection = findConnection(row.uid, row.lextree.get(), conn);
                row.error = lexpr::Error::NoError;
                row.error_text = "Ошибок нет";
                continue;
            }
        }
        if ( row.operation == lcheck::given ) {
            if ( !node_p_tool::pair_equal( row.ltree.get(), row.rtree.get(), mGivens.begin(), mGivens.end(),
                                         &ExtraData::ltree, &ExtraData::rtree ) ) {
                row.error = lexpr::Error::OperationError;
                row.error_text = "Некорректно указана операция или она не элементарна";
                continue;
            }
            else {
                row.connection = findConnection(row.uid, row.lextree.get(), conn);
                row.error = lexpr::Error::NoError;
                row.error_text = "Ошибок нет";
                continue;
            }
        }
        if ( row.operation == lcheck::unknown ) {
            if ( !node_p_tool::pair_equal( row.ltree.get(), row.rtree.get(), mUnknowns.begin(), mUnknowns.end(),
                                         &ExtraData::ltree, &ExtraData::rtree ) ) {
                row.error = lexpr::Error::OperationError;
                row.error_text = "Некорректно указана операция или она не элементарна";
                continue;
            }
            else {
                row.connection = findConnection(row.uid, row.lextree.get(), conn);
                row.error = lexpr::Error::NoError;
                row.error_text = "Ошибок нет";
                continue;
            }
        }
        if ( row.operation == lcheck::answer ) {
            if ( !node_p_tool::pair_equal( row.ltree.get(), row.rtree.get(), mAnswer.begin(), mAnswer.end(),
                                         &ExtraData::ltree, &ExtraData::rtree) ) {
                row.error = lexpr::Error::OperationError;
                row.error_text = "Некорректно указана операция или она не элементарна";
                continue;
            }
            else {
                row.connection = findConnection(row.uid, row.rextree.get(), conn);
                row.error = lexpr::Error::NoError;
                row.error_text = "Ошибок нет";
                continue;
            }
        }
        if ( row.operation == lcheck::conclusion ) {
            auto lconnection = -1;
            auto rconnection = -2;
            for ( auto& lrow: mRows ) {
                if (lrow.uid == row.uid) break;
                if (lrow.error != lexpr::Error::NoError) continue;
                if (node_p_tool::equal(row.lextree.get(), lrow.lextree.get()))
                    lconnection = lrow.connection;
            }
            for ( auto& lrow: mRows ) {
                if (lrow.uid == row.uid) break;
                if (lrow.error != lexpr::Error::NoError) continue;
                if (node_p_tool::equal(row.rextree.get(), lrow.rextree.get()))
                    rconnection = lrow.connection;
            }
            if (lconnection != rconnection) {
                row.error = lexpr::Error::OperationError;
                row.error_text = lformat::format("Выражение не является выводом ($$:$$)",
                                                 lconnection, rconnection);
                continue;
            }
            else {
                row.connection = lconnection;
                row.error = lexpr::Error::NoError;
                row.error_text = "Ошибок нет";
                continue;
            }
        }
    }
mLogger.Log(LLogger::Level::Medium, "Not released $$ nodes", AllacationCount_Node);
    LRow* unknown = nullptr;
    LRow* answer = nullptr;
    for ( auto& row: mRows ) {
        if ( row.error == lexpr::Error::NoCheck ) continue;
        if (row.operation == lcheck::unknown) unknown = &row;
        if (row.operation == lcheck::answer) answer = &row;
    }
    if (unknown && answer && (unknown->connection == answer->connection)) {
        mIsCompleted = true;
    }
}

std::string LTask::Result()
{
    std::string result;
    result += "{\"errors\": [";
    for ( auto& row: mRows ) {
        result += "{ ";
            result += "\"uid\": " + std::to_string( row.uid ) + ", ";
            result += "\"error\": " + std::to_string( static_cast<int>(row.error) ) + ", ";
            result += "\"error_text\": \"" + row.error_text + "\", ";
            result += "\"connection\": " + std::to_string(row.connection);
        result += " },";
    }
    if ( mRows.empty() ) result += "]";
    else result.back() = ']';
    result += ", \"result\": ";
    result += (mIsCompleted ? "true }" :  "false }");
    return result;
}

int LTask::findConnection(int uid, const lexpr::tree_e* tree, int &conn)
{
    auto connection = -1;
    for ( auto& row: mRows ) {
        if (row.uid == uid) break;
        if (row.error != lexpr::Error::NoError) continue;
        if (node_p_tool::equal(row.rextree.get(), tree))
            connection = row.connection;
    }
    return connection == -1 ? ++conn : connection;
}
