// Threads on the Vita (VitaSDK's pthread-embedded under libstdc++).
//
// Picori uses std::thread, std::async and std::mutex: the asset extraction
// runs on worker threads, and the music is mixed on SDL's audio thread under
// std::mutex. So libstdc++'s threads must be on:
//   - CMakeLists.txt links with -Wl,-u,pthread_cancel, the symbol libstdc++
//     checks to decide that threads exist;
//   - pthread-embedded must be initialised before anything uses it. Its own
//     constructor runs at priority 101, but libstdc++ sets up its locale and
//     iostreams earlier (priority 0) and takes a pthread_once there, so
//     pthread_init() is called here first: this object is linked before
//     libstdc++, so its priority-0 constructor runs before libstdc++'s.
//   - threads made without attributes (std::thread) get 1 MiB of stack
//     instead of pthread-embedded's small default (-Wl,--wrap=pthread_create).
#include <pthread.h>
#include <stdio.h>

#include <exception>
#include <mutex>
#include <thread>

#pragma GCC diagnostic ignored "-Wprio-ctor-dtor"

__attribute__((constructor(0))) static void tmc_vita_threads_early(void)
{
    pthread_init();
}

extern "C" int __real_pthread_create(pthread_t* th, const pthread_attr_t* attr, void* (*fn)(void*), void* arg);
extern "C" int __wrap_pthread_create(pthread_t* th, const pthread_attr_t* attr, void* (*fn)(void*), void* arg)
{
    if (attr)
        return __real_pthread_create(th, attr, fn, arg);
    pthread_attr_t big;
    pthread_attr_init(&big);
    pthread_attr_setstacksize(&big, 1u << 20);
    const int r = __real_pthread_create(th, &big, fn, arg);
    pthread_attr_destroy(&big);
    return r;
}

// After vita_start.c has opened the log: does std::thread work? (the log
// says so, or why not)
static int sOnceRuns = 0;
static void tmc_vita_once(void) { sOnceRuns++; }

__attribute__((constructor(110))) static void tmc_vita_threads_check(void)
{
    // pthread_once: libstdc++ sets up its own locks through it
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    const int onceResult = pthread_once(&once, tmc_vita_once);
    fprintf(stderr, "[vita] pthread_once: result %d, ran %d time(s)\n", onceResult, sOnceRuns);
    fprintf(stderr, "[vita] threads: testing std::thread\n");
    try {
        int value = 0;
        std::mutex m;
        std::thread t([&] {
            std::lock_guard<std::mutex> lock(m);
            value = 42;
        });
        t.join();
        fprintf(stderr, "[vita] threads: %s\n", value == 42 ? "working" : "did not run");
    } catch (const std::exception& e) {
        fprintf(stderr, "[vita] threads: NOT working (%s)\n", e.what());
    }
}
