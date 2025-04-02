#include <windows.h>
#include <chrono>
#include <iostream>
#include <intrin.h>
#include <vector>
#include <cmath>
#include <algorithm>

// Measure system time (CLOCK_REALTIME analog)
void measureSystemTime() {
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    std::cout << "System time (100ns): " << uli.QuadPart << std::endl;
}

// Measure CLOCK_REALTIME analog
void measureRealtimeClock() {
    FILETIME ft1, ft2;
    const int N = 1000; 
    double sum = 0.0;
    double sum_sq = 0.0;

    for (int i = 0; i < N; ++i) {
        GetSystemTimePreciseAsFileTime(&ft1);
        GetSystemTimePreciseAsFileTime(&ft2);

        // 100 ns interval
        ULARGE_INTEGER uli1, uli2;
        uli1.LowPart = ft1.dwLowDateTime;
        uli1.HighPart = ft1.dwHighDateTime;
        uli2.LowPart = ft2.dwLowDateTime;
        uli2.HighPart = ft2.dwHighDateTime;

        double diff = static_cast<double>(uli2.QuadPart - uli1.QuadPart) * 100.0; // ns
        sum += diff;
        sum_sq += diff * diff;
    }

    double mean = sum / N; 
    double std_dev = sqrt((sum_sq - sum * sum / N) / (N - 1));

    std::cout << "CLOCK_REALTIME (analog):\n"
              << "  Average resolution: " << mean << " ns\n"
              << "  Error rate: +-" << std_dev << " ns\n";
}

// Measure Performance Counter (HPET analog) with error calculation
void measurePerformanceCounter() {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    
    const int N = 1000;
    std::vector<double> samples(N);
    double sum = 0.0;
    double sum_sq = 0.0;

    for (int i = 0; i < N; ++i) {
        LARGE_INTEGER start, end;
        QueryPerformanceCounter(&start);
        QueryPerformanceCounter(&end);
        
        double diff = (end.QuadPart - start.QuadPart) * 1e6 / freq.QuadPart; // microseconds
        samples[i] = diff;
        sum += diff;
        sum_sq += diff * diff;
    }

    double mean = sum / N;
    double std_dev = sqrt((sum_sq - sum * sum / N) / (N - 1));

    std::cout << "Performance Counter:\n"
              << "  Average resolution: " << mean << " mks\n"
              << "  Resolution error: +-" << std_dev << " mks\n"
              << "  Min: " << *min_element(samples.begin(), samples.end()) << " mks\n"
              << "  Max: " << *max_element(samples.begin(), samples.end()) << " mks\n";
}

// Measure RDTSC resolution with error calculation
void measureRDTSC() {
    const int N = 1000;
    std::vector<unsigned long long> samples(N);
    double sum = 0.0;
    double sum_sq = 0.0;

    for (int i = 0; i < N; ++i) {
        unsigned long long start = __rdtsc();
        unsigned long long end = __rdtsc();
        
        unsigned long long diff = end - start;
        samples[i] = diff;
        sum += diff;
        sum_sq += static_cast<double>(diff) * diff;
    }

    double mean = sum / N;
    double std_dev = sqrt((sum_sq - sum * sum / N) / (N - 1));

    std::cout << "RDTSC:\n"
              << "  Average resolution: " << mean << " cycles\n"
              << "  Resolution error: +-" << std_dev << " cycles\n"
              << "  Min: " << *min_element(samples.begin(), samples.end()) << " cycles\n"
              << "  Max: " << *max_element(samples.begin(), samples.end()) << " cycles\n";
}

// Calibrate RDTSC with error calculation
void calibrateRDTSC() {
    const int N = 10;  // Reduced number due to long measurement time
    std::vector<double> samples(N);
    double sum = 0.0;
    double sum_sq = 0.0;
    
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    for (int i = 0; i < N; ++i) {
        LARGE_INTEGER start, end;
        QueryPerformanceCounter(&start);
        unsigned long long tsc_start = __rdtsc();
        Sleep(100);  // Reduced from 1000ms to 100ms for faster testing
        QueryPerformanceCounter(&end);
        unsigned long long tsc_end = __rdtsc();
        
        double sec = (end.QuadPart - start.QuadPart) / static_cast<double>(freq.QuadPart);
        double cycles_per_sec = (tsc_end - tsc_start) / sec;
        samples[i] = cycles_per_sec;
        sum += cycles_per_sec;
        sum_sq += cycles_per_sec * cycles_per_sec;
    }

    double mean = sum / N;
    double std_dev = sqrt((sum_sq - sum * sum / N) / (N - 1));

    std::cout << "RDTSC Calibration:\n"
              << "  Average frequency: " << mean << " Hz (" << mean/1e9 << " GHz)\n"
              << "  Calibration error: +-" << std_dev << " Hz (+-" << std_dev/1e9 << " GHz)\n"
              << "  Min: " << *min_element(samples.begin(), samples.end())/1e9 << " GHz\n"
              << "  Max: " << *max_element(samples.begin(), samples.end())/1e9 << " GHz\n";
}

int main() {
    std::cout << "===== Measure time =====" << std::endl;
    measureSystemTime();
    measureRealtimeClock();
    measurePerformanceCounter();
    measureRDTSC();
    calibrateRDTSC();
    return 0;
}