/*
Copyright (C) 2016 Denso IT Laboratory, Inc.
All Rights Reserved

Denso IT Laboratory, Inc. retains sole and exclusive ownership of all
intellectual property rights including copyrights and patents related to this
Software.

Permission is hereby granted, free of charge, to any person obtaining a copy
of the Software and accompanying documentation to use, copy, modify, merge,
publish, or distribute the Software or software derived from it for
non-commercial purposes, such as academic study, education and personal use,
subject to the following conditions:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <vector>
#include <limits>
#include <sstream>
#include <iostream>
#include <iomanip>

// Benchmark the operation 'op'. The number of iterations refers to
// how many times the operation is run for each time measurement, the
// result is the minimum over a number of samples runs. The result is the
// amount of time in seconds for one iteration.
#ifdef _WIN32

union _LARGE_INTEGER;
typedef union _LARGE_INTEGER LARGE_INTEGER;
extern "C" int __stdcall QueryPerformanceCounter(LARGE_INTEGER*);
extern "C" int __stdcall QueryPerformanceFrequency(LARGE_INTEGER*);

template <typename F>
double benchmark_median(int samples, F op) {
    int64_t freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    double best = std::numeric_limits<double>::infinity();
    for (int i = 0; i < samples; ++i) {
        int64_t t1;
        QueryPerformanceCounter((LARGE_INTEGER*)&t1);

        op();

        int64_t t2;
        QueryPerformanceCounter((LARGE_INTEGER*)&t2);
        double dt = (t2 - t1) / static_cast<double>(freq);
        if (dt < best) best = dt;
    }
    return best / iterations;
}

#else

#include <chrono>
#include <fstream>

template <typename F>
double benchmark_median(const std::string &path, int samples, F op) {
    std::vector<double> result(samples);

    for (int i = 0; i < samples; ++i) {
        auto t1 = std::chrono::high_resolution_clock::now();

        op();

        auto t2 = std::chrono::high_resolution_clock::now();

        // convers usec -> sec
        double dt = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6;

        result[i] = dt;
    }

    // sort result
    double median;
    size_t size = result.size();
    std::sort(result.begin(), result.end());

    // export result
    std::ofstream fs(path, std::ios::out|std::ios::trunc);
    for (int idx = 0; idx < samples; ++idx) {
      fs << std::setprecision(std::numeric_limits<double>::digits10 + 1) << result[idx] << '\n';
    }
    fs.close();

    if (size  % 2 == 0) {
      median = (result[size / 2 - 1] + result[size / 2]) / 2;
    } else {
      median = result[size / 2];
    }

    return median;
}

#endif

#endif
