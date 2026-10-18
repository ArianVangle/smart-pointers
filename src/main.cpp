#include <string>

#include "Benchmark.hpp"
#include "Tests.hpp"
#include "Ui.hpp"

int main(int argc, char** argv) {
    std::string mode = argc > 1 ? argv[1] : "";
    if (mode == "tests") {
        RunAllTests();
    } else if (mode == "benchmark") {
        bool quick = argc > 2 && std::string(argv[2]) == "quick";
        return RunBenchmark(quick) ? 0 : 1;
    } else {
        RunUi();
    }
    return 0;
}
