#include <windows.h>
#include <chrono>
#include <iostream>
#include <intrin.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace std;

struct TimerMetrics {
    double resolution;
    double resolution_error;
    double a1; // Syscall entry time 
    double a2; // Syscall exit time
};

// Measure timer latency for the timer
template<typename TimerFunc>
TimerMetrics measureTimerLatency(TimerFunc timer, const string& name) {
    const int N = 10000;
    vector<double> total_times(N);
    vector<double> t1(N), t2(N);

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    // Measure general time of consistent calls
    for (int i = 0; i < N; ++i) {
        LARGE_INTEGER start, end;
        QueryPerformanceCounter(&start);
        timer(); // First call
        timer(); // Second call
        QueryPerformanceCounter(&end);
        
        total_times[i] = (end.QuadPart - start.QuadPart) * 1e6 / freq.QuadPart; // mks
    }

    // Measure statistics
    double avg_total = accumulate(total_times.begin(), total_times.end(), 0.0) / N;
    double a_avg = avg_total / 2; // A1 + A2

    // A1 ~= A2
    TimerMetrics metrics;
    metrics.a1 = a_avg / 2;
    metrics.a2 = a_avg / 2;

    // Measure timer resolution
    vector<double> resolutions(N);
    for (int i = 0; i < N; ++i) {
        LARGE_INTEGER start, end;
        timer();
        QueryPerformanceCounter(&start);
        timer();
        QueryPerformanceCounter(&end);
        resolutions[i] = (end.QuadPart - start.QuadPart) * 1e6 / freq.QuadPart;
    }

    double res_mean = accumulate(resolutions.begin(), resolutions.end(), 0.0) / N;
    double sq_sum = inner_product(resolutions.begin(), resolutions.end(), resolutions.begin(), 0.0);
    metrics.resolution = res_mean;
    metrics.resolution_error = sqrt((sq_sum - N * res_mean * res_mean) / (N - 1));

    return metrics;
}

// Wrapper functions of each timer
void callSystemTime() {
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
}

void callPerformanceCounter() {
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
}

void callRDTSC() {
    __rdtsc();
}

int main() {
    // Measure data of all the timers
    TimerMetrics system_time = measureTimerLatency(callSystemTime, "System Time");
    TimerMetrics perf_counter = measureTimerLatency(callPerformanceCounter, "Performance Counter");
    TimerMetrics rdtsc = measureTimerLatency(callRDTSC, "RDTSC");

    // Print the result as table
    cout << "| Timer Name           | Resolution (μs) | Resolution Error | A1 (mks) | A2 (mks) |\n";
    cout << "|----------------------|------------------|------------------|---------|---------|\n";
    printf("| %-20s | %-16.3f | %-16.3f | %-7.3f | %-7.3f |\n", 
           "System Time", system_time.resolution, system_time.resolution_error, system_time.a1, system_time.a2);
    printf("| %-20s | %-16.3f | %-16.3f | %-7.3f | %-7.3f |\n", 
           "Performance Counter", perf_counter.resolution, perf_counter.resolution_error, perf_counter.a1, perf_counter.a2);
    printf("| %-20s | %-16.3f | %-16.3f | %-7.3f | %-7.3f |\n", 
           "RDTSC", rdtsc.resolution, rdtsc.resolution_error, rdtsc.a1, rdtsc.a2);

    // Calibrate RDTSC
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    unsigned long long tsc_start = __rdtsc();
    Sleep(100);
    unsigned long long tsc_end = __rdtsc();
    double cycles_per_sec = (tsc_end - tsc_start) / 0.1; // 100ms = 0.1s
    
    cout << "\nRDTSC Calibration:\n";
    cout << "  CPU Frequency: " << cycles_per_sec / 1e9 << " GHz\n";

    return 0;
}