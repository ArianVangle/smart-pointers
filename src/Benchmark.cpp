#include "Benchmark.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "DynamicArray.hpp"
#include "HeapCounter.hpp"
#include "ShrdPtr.hpp"
#include "UnqPtr.hpp"

namespace {

struct Obj {
    long a;
    long b;
    explicit Obj(long v) : a(v), b(v) {}
};

struct Result {
    double createMs = 0;
    double accessMs = 0;
    double destroyMs = 0;
    double memoryKb = 0;
    double allocationsPerObject = 0;

    double TotalMs() const { return createMs + accessMs + destroyMs; }
};

using Clock = std::chrono::steady_clock;

double Ms(Clock::time_point from, Clock::time_point to) {
    return std::chrono::duration<double, std::milli>(to - from).count();
}

const int kRounds = 3;

size_t Repeats(size_t n) {
    const size_t objectsPerMeasurement = 1000000;
    return n >= objectsPerMeasurement ? 1 : objectsPerMeasurement / n;
}

template <typename Ptr, typename Make, typename Free>
Result Run(size_t n, Make make, Free release) {
    Result result;
    size_t repeats = Repeats(n);
    long sum = 0;
    for (size_t r = 0; r < repeats; ++r) {
        size_t bytesBefore = HeapBytes();
        size_t allocationsBefore = HeapAllocations();
        auto start = Clock::now();
        Clock::time_point created;
        Clock::time_point accessed;
        {
            DynamicArray<Ptr> arr;
            for (size_t i = 0; i < n; ++i) {
                arr.Append(make(static_cast<long>(i)));
            }
            created = Clock::now();
            if (r == 0) {
                result.memoryKb =
                    static_cast<double>(HeapBytes() - bytesBefore) / 1024.0;
                result.allocationsPerObject =
                    static_cast<double>(HeapAllocations() - allocationsBefore) /
                    static_cast<double>(n);
            }
            for (size_t i = 0; i < n; ++i) {
                sum += arr[i]->a;
            }
            accessed = Clock::now();
            for (size_t i = 0; i < n; ++i) {
                release(arr[i]);
            }
        }
        auto end = Clock::now();
        result.createMs += Ms(start, created);
        result.accessMs += Ms(created, accessed);
        result.destroyMs += Ms(accessed, end);
    }
    if (sum < 0) {
        std::cout << sum;
    }
    result.createMs /= static_cast<double>(repeats);
    result.accessMs /= static_cast<double>(repeats);
    result.destroyMs /= static_cast<double>(repeats);
    return result;
}

Result RunRaw(size_t n) {
    return Run<Obj*>(
        n, [](long v) { return new Obj(v); }, [](Obj*& p) { delete p; });
}

Result RunUnq(size_t n) {
    return Run<UnqPtr<Obj>>(
        n, [](long v) { return UnqPtr<Obj>(new Obj(v)); },
        [](UnqPtr<Obj>&) {});
}

Result RunStdUnique(size_t n) {
    return Run<std::unique_ptr<Obj>>(
        n, [](long v) { return std::make_unique<Obj>(v); },
        [](std::unique_ptr<Obj>&) {});
}

Result RunShrd(size_t n) {
    return Run<ShrdPtr<Obj>>(
        n, [](long v) { return ShrdPtr<Obj>(UnqPtr<Obj>(new Obj(v))); },
        [](ShrdPtr<Obj>&) {});
}

Result RunStdShrd(size_t n) {
    return Run<std::shared_ptr<Obj>>(
        n, [](long v) { return std::make_shared<Obj>(v); },
        [](std::shared_ptr<Obj>&) {});
}

const int kMethods = 5;
const char* const kNames[kMethods] = {"raw", "UnqPtr", "std_unique", "ShrdPtr",
                                      "std_shared"};
Result (*const kRunners[kMethods])(size_t) = {RunRaw, RunUnq, RunStdUnique,
                                              RunShrd, RunStdShrd};

DynamicArray<size_t> MakeSizes(bool quick) {
    DynamicArray<size_t> sizes;
    size_t last = quick ? 100000 : 10000000;
    for (size_t n = 10; n <= last; n *= 10) {
        sizes.Append(n);
    }
    return sizes;
}

void PrintHeader() {
    std::cout << std::left << std::setw(12) << "method" << std::setw(11) << "n"
              << std::setw(13) << "create_ms" << std::setw(13) << "access_ms"
              << std::setw(13) << "destroy_ms" << std::setw(13) << "total_ms"
              << std::setw(13) << "memory_kb" << "allocs/obj\n";
}

void PrintRow(const std::string& name, size_t n, const Result& r) {
    std::cout << std::left << std::setw(12) << name << std::setw(11) << n
              << std::fixed << std::setprecision(4) << std::setw(13)
              << r.createMs << std::setw(13) << r.accessMs << std::setw(13)
              << r.destroyMs << std::setw(13) << r.TotalMs()
              << std::setprecision(1) << std::setw(13) << r.memoryKb
              << std::setprecision(2) << r.allocationsPerObject << "\n";
}

void WriteRow(std::ofstream& csv, const std::string& name, size_t n,
              const Result& r) {
    csv << name << "," << n << "," << r.createMs << "," << r.accessMs << ","
        << r.destroyMs << "," << r.TotalMs() << "," << r.memoryKb << ","
        << r.allocationsPerObject << "\n";
}

void PrintPointerSizes() {
    std::cout << "\nsizeof(Obj*)             = " << sizeof(Obj*)
              << "\nsizeof(UnqPtr<Obj>)      = " << sizeof(UnqPtr<Obj>)
              << "\nsizeof(unique_ptr<Obj>)  = " << sizeof(std::unique_ptr<Obj>)
              << "\nsizeof(ShrdPtr<Obj>)     = " << sizeof(ShrdPtr<Obj>)
              << "\nsizeof(shared_ptr<Obj>)  = " << sizeof(std::shared_ptr<Obj>)
              << "\n";
}

} // namespace

bool RunBenchmark(bool quick) {
    std::ofstream csv("results/results.csv");
    if (!csv) {
        std::cout << "не удалось открыть results/results.csv: запускайте "
                     "программу из корня проекта\n";
        return false;
    }
    csv << std::setprecision(10);
    csv << "method,n,create_ms,access_ms,destroy_ms,time_ms,memory_kb,"
           "allocations_per_object\n";

    DynamicArray<size_t> sizes = MakeSizes(quick);
    PrintHeader();
    for (size_t k = 0; k < sizes.GetLength(); ++k) {
        size_t n = sizes[k];
        Result results[kMethods];
        for (int round = 0; round < kRounds; ++round) {
            for (int i = 0; i < kMethods; ++i) {
                Result current = kRunners[i](n);
                if (round == 0 || current.TotalMs() < results[i].TotalMs()) {
                    results[i] = current;
                }
            }
        }
        for (int i = 0; i < kMethods; ++i) {
            PrintRow(kNames[i], n, results[i]);
            WriteRow(csv, kNames[i], n, results[i]);
        }
        std::cout.flush();
    }
    PrintPointerSizes();
    std::cout << "результаты сохранены в results/results.csv" << std::endl;
    std::cout << std::defaultfloat;
    return true;
}
