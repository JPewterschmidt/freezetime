#include <time.h>
#include <dlfcn.h>
#include <mutex>
#include <memory>
#include <iostream>
#include <string>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/time.h>
#include <gnu/lib-names.h>

namespace
{
    ::std::once_flag s_once_flag;
    time_t s_fixed_time;

    ::timespec s_ts;

    ::tms s_times_time;
    ::clock_t s_times_result;

    ::std::string s_myname;
    auto s_dl_closer = [](void* ptr){ ::dlclose(ptr); };
    using handle_t = ::std::unique_ptr<void, decltype(s_dl_closer)>; 

    handle_t open_libc()
    {
        return { ::dlopen(LIBC_SO, RTLD_LAZY), s_dl_closer };
    }

    void* get_func(handle_t& h, const char* name)
    {
        return ::dlsym(h.get(), name);
    }
}


static void init()
{
    handle_t handle = open_libc();
    if (!handle) return;

    // libc `time()` 
    auto libc_clock_gettime = (int (*) (clock_t, ::timespec*))get_func(handle, "clock_gettime");
    if (!libc_clock_gettime) return;
    
    libc_clock_gettime(CLOCK_REALTIME, &s_ts);

    ::Dl_info dl_info{};
    // Get current shared object name.
    if (::dladdr((void*)&init, &dl_info))
    {
        s_myname = dl_info.dli_fname;
    }

    auto libc_times = (clock_t (*) (::tms*))get_func(handle, "times");
    if (!libc_times) return;
    
    s_times_result = libc_times(&s_times_time);   
}

static ::timespec time_to_timespec(time_t t)
{
    return { .tv_sec = t };
}

// Hooks ------------------------------------------------------------------------

time_t time(time_t* tloc)
{
    /*XXX init*/ ::std::call_once(s_once_flag, init);
    if (tloc) *tloc = s_ts.tv_sec;
    return s_ts.tv_sec;
}

int clock_gettime(::clockid_t clkid, ::timespec* tp)
{
    /*XXX init*/ ::std::call_once(s_once_flag, init);
    switch (clkid)
    {
        case CLOCK_REALTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_REALTIME_COARSE:
        case CLOCK_MONOTONIC:
        case CLOCK_MONOTONIC_COARSE:
        case CLOCK_TAI:
            if (tp) *tp = s_ts;
            break;       
        default: 
            ::std::cerr << (s_myname.size() ? s_myname : "freezetime") 
                        << ": clock_gettime() operation not support. clkid = " 
                        << clkid 
                        << ". clock_gettime will return a EINVAL."
                        << ::std::endl;
            errno = EINVAL;
            return -1;
    }
    return 0;
}

int ftime(::timeb* tp)
{
    /*XXX init*/ ::std::call_once(s_once_flag, init);
    if (tp) *tp = { .time = s_ts.tv_sec, .millitm = static_cast<unsigned short>(s_ts.tv_nsec / 1'000'000) };
    return 0;
}

int gettimeofday(::timeval* tv, [[maybe_unused]] void* /*::timezone* */)
{
    /*XXX init*/ ::std::call_once(s_once_flag, init);
    if (tv) *tv = { .tv_sec = s_ts.tv_sec, .tv_usec = s_ts.tv_nsec / 1000 };
    return 0;
}

::clock_t times(::tms* buf)
{
    /*XXX init*/ ::std::call_once(s_once_flag, init);
    if (buf) *buf = s_times_time;
    return s_times_result;
}
