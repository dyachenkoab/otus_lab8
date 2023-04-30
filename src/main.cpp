#include <iostream>
#include <boost/program_options.hpp>
#include "strategy.h"
#include "processor.h"
#include "handler.h"

namespace po = boost::program_options;

int main(int argc, const char *argv[])
{
    Processor processor;

    try {
        po::options_description desc { "Options" };
        desc.add_options()("help,h", "Help screen")
        ("scanDir,s", po::value<string>(), "Scan directory")
        ("exDir,e", po::value<string>(), "Exclude directory from scan")
        ("level,l", po::value<int>()->default_value(0)->notifier([&processor](int level) {
            int lev = level;
            if (lev < 0 || lev > 1) {
                std::cout << "Wrong level. Zero level will be used" << '\n';
                lev = 0;
            }
            processor.scanLevel(lev);
         }), "Scan level")
         ("min,m", po::value<int>()->default_value(1), "Minimal file size (at bytes) to scan")
         ("mask", po::value<string>(),"Mask")
         ("blockSize,b", po::value<size_t>()->default_value(5)->notifier([&processor](size_t size) {
            processor.blockSize(size);
          }),"Size of block (at bytes) to compare")
         ("hash", po::value<string>()->default_value("default")->notifier([&processor](const string &hash) {
            processor.hashAlgo(hash);
          }),
          "Hash algorithm, can be default or crc32");

        po::variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {
            std::cout << desc << '\n';
            return 0;
        }

        if (vm.count("scanDir")) {
            processor.scanDirs(vm["scanDir"].as<string>());
        } else {
            std::cout << "There is no directory for scan" << std::endl;
            return 0;
        }

        if (vm.count("exDir")) {
            processor.pushRequest(unique_ptr<ExcludedRequest>(new ExcludedRequest(vm["exDir"].as<string>())));
        }

        if (vm.count("min")) {
            processor.pushRequest(unique_ptr<SizeRequest>(new SizeRequest(vm["min"].as<int>())));
        }

        if (vm.count("mask")) {
            processor.pushRequest(unique_ptr<MaskRequest>(new MaskRequest(vm["mask"].as<string>())));
        }

        processor.checkFile();

    } catch (const po::error &ex) {
        std::cerr << ex.what() << '\n';
    }

    return 0;
}
