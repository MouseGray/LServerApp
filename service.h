#pragma once

#include "database.h"
#include "ltask.h"

namespace service {

    std::string execute(const char *data , LLogger &logger);

    std::string getTaskUserText(const std::string &uuid, int taskid );

    std::tuple<std::vector<char>, std::string, std::string, std::string> getTaskStaticData(int taskid);

};
