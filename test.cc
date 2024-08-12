#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/times.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring> // memcmp

using namespace ::std::chrono_literals;

namespace
{
    auto tests = { 
        +[] 
        {
            ::timeval tv1{}, tv2{};

            const int ret1 = ::gettimeofday(&tv1, nullptr);
            ::std::this_thread::sleep_for(1ms);
            const int ret2 = ::gettimeofday(&tv2, nullptr);

            return !ret1 && !ret2 && !::std::memcmp(&tv1, &tv2, sizeof(tv1));
        },

        +[]
        {
            ::timespec t1{}, t2{};
            
            const int ret1 = ::clock_gettime(CLOCK_REALTIME, &t1);
            ::std::this_thread::sleep_for(1ms);
            const int ret2 = ::clock_gettime(CLOCK_REALTIME, &t2);

            return !ret1 && !ret2 && !::std::memcmp(&t1, &t2, sizeof(t1));
        },

        +[]
        {
            const time_t t1 = time(nullptr);
            ::std::this_thread::sleep_for(1s);

            time_t t2{}; 
            time(&t2);
            
            return t1 && (t1 == t2);
        },

        +[]
        {
            ::tms t1{}, t2{};
            ::clock_t c1{}, c2{};
            c1 = times(&t1);
            ::std::this_thread::sleep_for(1ms);
            c2 = times(&t2);
            
            return (c1 == c1) && !::std::memcmp(&t1, &t2, sizeof(t1));
        },
    };
}

int main()
{
    for (auto& test : tests)
    {
        if (!test()) 
        {
            ::std::cerr << "test failed" << ::std::endl;
            return 1;
        }
    }

    ::std::cout << "test succeed" << ::std::endl;
    return 0;
}
