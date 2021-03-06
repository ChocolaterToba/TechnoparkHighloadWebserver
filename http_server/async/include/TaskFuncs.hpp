#pragma once

#include <functional>
#include <vector>
#include <map>
#include <string>
#include "HTTPClient.hpp"

typedef std::function<void(std::map<std::string, std::string>& headers,
                           std::shared_ptr<std::vector<char>> body,
                           HTTPClient& input, HTTPClient& output)> MainFuncType;
MainFuncType PreProcess(std::map<std::string, std::string>& headers,
                        std::shared_ptr<std::vector<char>> data,
                        HTTPClient& input);
void MainProcessBasic(std::map<std::string, std::string>& headers,
                      std::shared_ptr<std::vector<char>> body,
                      HTTPClient& input, HTTPClient& output);
void PostProcess(std::map<std::string, std::string>& headers,
                 std::shared_ptr<std::vector<char>> body,
                 HTTPClient& output);
