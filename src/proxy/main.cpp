/**
 *   (C) Copyright 2016 Mirantis, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may
 *   not use this file except in compliance with the License. You may obtain
 *   a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */


#include <iostream>
#include <thread>
#include <easylogging++.h>
#include <boost/program_options.hpp>

#include "centralproxy.h"
#include "common/configuration.h"


INITIALIZE_EASYLOGGINGPP


int main(int argc, char* argv[])
{
    START_EASYLOGGINGPP(argc, argv);
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "%level %datetime{%H:%m:%s} (%func): %msg");
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

    namespace po = boost::program_options;
    po::options_description desc("ZeroMQ proxy service");
    std::string config_file;
    desc.add_options()
      ("help,h", "Show help")
      ("config-file,c", po::value<std::string>(&config_file), "ZeroMQ proxy configuration file");
    po::variables_map vm;

    try
    {
        po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
        po::store(parsed, vm);
        po::notify(vm);

        zmqproxy::Configuration config(config_file);
        zmqproxy::CentralProxy proxy(config);

        while (true)
        {
            proxy.pollForMessages();
        }

    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
    }

	return 0;
}
