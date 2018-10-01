#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>

#include "renoster/plugin.h"
#include "renoster/reno.h"
#include "renoster/renoparser.h"

namespace po = boost::program_options;

using namespace renoster;

int main(int argc, char * argv[]) {
    int nthreads = 1;
    std::vector<std::string> filenames;

    po::options_description generic("Generic options");
    generic.add_options()
        ("version,v", "print version string")
        ("help,h", "produce help message");

    po::options_description rendering("Rendering options");
    rendering.add_options()
        ("nthreads", po::value<int>(&nthreads), "set number of threads used");

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("filenames", po::value<std::vector<std::string>>(&filenames));

    po::positional_options_description positionals;
    positionals.add("filenames", -1);

    po::options_description options;
    options.add(generic);
    options.add(rendering);
    options.add(hidden);

    po::variables_map vm;

    try
    {
        po::command_line_parser parser(argc, argv);
        parser.options(options);
        parser.positional(positionals);
        po::store(parser.run(), vm);
    }
    catch (po::error & e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }

    po::notify(vm);

    if (vm.count("help"))
    {
        po::options_description cmdlineOptions;
        cmdlineOptions.add(generic);
        cmdlineOptions.add(rendering);
        std::cout << cmdlineOptions << std::endl;
        return 1;
    }

    for (auto && filename : filenames)
    {
        SetPluginSearchPath(".");
        RenoBegin();
        ParseRenoFile(filename);
        RenoEnd();
    }

    return 0;
}
