#include <windows.h>
#include <chrono>
#include <iostream>
#include <intrin.h>

// Measure system time (CLOCK_REALTIME analog)
void measureSystemTime() {
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    std::cout << "System time (100ns): " << uli.QuadPart << std::endl;
}

// Measure Perfomance Counter (HPET analog)
void measurePerformanceCounter() {
    LARGE_INTEGER freq, start, end;
    double resolution_sum;

    for (int i = 0; i < 1000; i++) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
        // Measure delay
        QueryPerformanceCounter(&end);
        resolution_sum += (end.QuadPart - start.QuadPart) * 1e6 / freq.QuadPart; // mks
    }
    std::cout << "Performance counter resolution: " << resolution_sum / 1000 << " mks" << std::endl;
}

// Measure RDTSC
void measureRDTSC() {
    unsigned long long start = __rdtsc();
    unsigned long long end = __rdtsc();
    std::cout << "RDTSC resolution: " << (end - start) << " cycles" << std::endl;
}

// Calibrate RDTSC
void calibrateRDTSC() {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    unsigned long long tsc_start = __rdtsc();
    Sleep(1000); // Waut 1 second
    QueryPerformanceCounter(&end);
    unsigned long long tsc_end = __rdtsc();
    double sec = (end.QuadPart - start.QuadPart) / static_cast<double>(freq.QuadPart);
    double cycles_per_sec = (tsc_end - tsc_start) / sec;
    std::cout << "RDTSC cycles/sec: " << cycles_per_sec << std::endl;
}

int main() {
    std::cout << "===== Measure time =====" << std::endl;
    measureSystemTime();
    measurePerformanceCounter();
    measureRDTSC();
    calibrateRDTSC();
    return 0;
}