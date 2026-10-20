// Function-local statics ("static T x = ...;" inside a function), made safe
// for threads by the compiler through __cxa_guard_acquire/release/abort.
//
// libstdc++'s versions lock one global recursive mutex that it sets up
// through pthread_once; on the Vita that mutex was never set up and the
// first local static the game reached (Dear ImGui's, in the start menu)
// crashed in pthread_mutex_lock on a null mutex. These replacements
// (-Wl,--wrap=__cxa_guard_*, CMakeLists.txt) use a recursive kernel mutex
// of the Vita's own instead, created on first use. The thread running a
// static's initialiser holds it until the initialiser finishes (as GCC's
// older ARM versions did), so a second thread waits and then sees the
// value ready.
//
// ARM EABI guard: a 32-bit word whose bit 0 means "initialised" (the
// compiler checks it itself before calling acquire).
#include <psp2/kernel/threadmgr.h>

namespace {

SceUID g_mutex = 0;

SceUID guard_mutex()
{
    SceUID m = __atomic_load_n(&g_mutex, __ATOMIC_ACQUIRE);
    if (m > 0)
        return m;
    const SceUID created = sceKernelCreateMutex("cxa_guard", SCE_KERNEL_MUTEX_ATTR_RECURSIVE, 0, nullptr);
    SceUID expected = 0;
    if (__atomic_compare_exchange_n(&g_mutex, &expected, created, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return created;
    sceKernelDeleteMutex(created); // another thread made it first
    return expected;
}

} // namespace

extern "C" int __wrap___cxa_guard_acquire(int* g)
{
    if (__atomic_load_n(g, __ATOMIC_ACQUIRE) & 1)
        return 0;
    sceKernelLockMutex(guard_mutex(), 1, nullptr);
    if (__atomic_load_n(g, __ATOMIC_ACQUIRE) & 1) {
        sceKernelUnlockMutex(guard_mutex(), 1);
        return 0;
    }
    return 1; // the caller runs the initialiser, then calls release (or abort)
}

extern "C" void __wrap___cxa_guard_release(int* g)
{
    __atomic_store_n(g, 1, __ATOMIC_RELEASE);
    sceKernelUnlockMutex(guard_mutex(), 1);
}

extern "C" void __wrap___cxa_guard_abort(int* g)
{
    (void)g;
    sceKernelUnlockMutex(guard_mutex(), 1);
}
