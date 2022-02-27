#pragma once

#include <map>
#include <array>
#include <string>
#include <vector>

#include <rapidjson/document.h>

#include <node.h>
#include <element.h>
#include <markercase.h>
#include <declaration.h>
#include <lvector.h>
#include <lvector_generator.h>

#include <lloggerlib.h>
#include <list>

#include "jsonutils.h"
#include "lexpression2.h"

struct LRow {
    LRow() {}

    LRow(const LRow&) = delete;
    LRow(LRow&&) = default;

    std::string left;
    std::string right;

    int operation;

    int uid;

    int connection;

    StdMarkerCase markers;

    std::unique_ptr< lexpr::tree_e > ltree;
    std::unique_ptr< lexpr::tree_e > rtree;

    std::unique_ptr< lexpr::tree_e > lextree;
    std::unique_ptr< lexpr::tree_e > rextree;

    std::array< int, 5 > numbers;

    lexpr::Error error;
    std::string error_text;
};

struct ExtraData {
    std::string left;
    std::string right;
    std::unique_ptr< lexpr::tree_e > ltree;
    std::unique_ptr< lexpr::tree_e > rtree;
};

class LTask
{
public:
    LTask( const char *data, LLogger& logger );
    void SetVariables( const std::vector< char >& variables );
    void SetAdditionData(const std::string &given, const std::string &unknown, const std::string& answer );
    void Deploy();
    void CheckAll();
    std::string Result();
private:
    int findConnection(int uid, const lexpr::tree_e* tree, int &conn);
    LRow& find(int uid);
    void fillExtraData(const char *json, std::vector< ExtraData >& extra_data);
    void deployExtraData(std::vector< ExtraData >& extra_data);
    std::pair<bool, char> validationData(const LRow& row);

    std::vector< LRow > mRows;
    std::vector< ExtraData > mGivens;
    std::vector< ExtraData > mUnknowns;
    std::vector< ExtraData > mAnswer;
    std::map< char, lexpr::lvector > mBaseValues;

    bool mIsCompleted = false;

    LLogger& mLogger;
};
