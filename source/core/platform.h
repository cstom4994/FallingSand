

#ifndef METADOT_PLATFORM_H
#define METADOT_PLATFORM_H

#include "core/core.h"
#include "core/macros.h"
#include "core/sdl_wrapper.h"
#include "core/stl/stl.h"

/*--------------------------------------------------------------------------
 * Platform specific headers
 *------------------------------------------------------------------------*/
#ifdef METADOT_PLATFORM_WINDOWS
#define WINDOWS_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#include <windows.h>
#elif defined(METADOT_PLATFORM_POSIX)
#include <pthread.h>
#include <sys/time.h>
#else
#error "Unsupported platform!"
#endif

#if defined(METADOT_PLATFORM_WINDOWS)
#include <Windows.h>
#include <io.h>
#include <shobjidl.h>

#if defined(_MSC_VER)
#define __func__ __FUNCTION__
#endif

#undef min
#undef max

#define PATH_SEP '\\'
#ifndef PATH_MAX
#define PATH_MAX 260
#endif

#elif defined(METADOT_PLATFORM_LINUX)
#include <bits/types/struct_tm.h>
#include <bits/types/time_t.h>
#include <dirent.h>
#include <limits.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#define PATH_SEP '/'
#elif defined(METADOT_PLATFORM_APPLE)
#include <TargetConditionals.h>
#include <mach-o/dyld.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#endif

#ifdef METADOT_PLATFORM_WINDOWS
#define getFullPath(a, b) GetFullPathName((LPCWSTR)a, MAX_PATH, (LPWSTR)b, NULL)
#define rmdir(a) _rmdir(a)
#define PATH_SEPARATOR '\\'
#else
#define getFullPath(a, b) realpath(a, b)
#define PATH_SEPARATOR '/'
#endif

#ifndef MAX_PATH
#include <limits.h>
#ifdef PATH_MAX
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 256
#endif
#endif

#if MAX_PATH > 1024
#undef MAX_PATH
#define MAX_PATH 1024
#endif

#define FS_LINE_INCR 256

/*--------------------------------------------------------------------------*/
static inline uint64_t getThreadID() {
#if defined(METADOT_PLATFORM_WINDOWS)
    return (uint64_t)GetCurrentThreadId();
#elif defined(METADOT_PLATFORM_LINUX)
    return (uint64_t)syscall(SYS_gettid);
#elif defined(METADOT_PLATFORM_APPLE)
    return (mach_port_t)::pthread_mach_thread_np(pthread_self());
#else
#error "Unsupported platform!"
#endif
}

// Thread Local Storage(TLS)
// msvc: https://learn.microsoft.com/en-us/cpp/parallel/thread-local-storage-tls

#ifdef METADOT_PLATFORM_WINDOWS

static inline uint32_t tlsAllocate() { return (uint32_t)TlsAlloc(); }

static inline void tlsSetValue(uint32_t _handle, void* _value) { TlsSetValue(_handle, _value); }

static inline void* tlsGetValue(uint32_t _handle) { return TlsGetValue(_handle); }

static inline void tlsFree(uint32_t _handle) { TlsFree(_handle); }

#else

static inline pthread_key_t tlsAllocate() {
    pthread_key_t handle;
    pthread_key_create(&handle, NULL);
    return handle;
}

static inline void tlsSetValue(pthread_key_t _handle, void* _value) { pthread_setspecific(_handle, _value); }

static inline void* tlsGetValue(pthread_key_t _handle) { return pthread_getspecific(_handle); }

static inline void tlsFree(pthread_key_t _handle) { pthread_key_delete(_handle); }

#endif

namespace MetaEngine {

#if defined(METADOT_PLATFORM_WINDOWS)
typedef CRITICAL_SECTION metadot_mutex;

static inline void metadot_mutex_init(metadot_mutex* _mutex) { InitializeCriticalSection(_mutex); }

static inline void metadot_mutex_destroy(metadot_mutex* _mutex) { DeleteCriticalSection(_mutex); }

static inline void metadot_mutex_lock(metadot_mutex* _mutex) { EnterCriticalSection(_mutex); }

static inline int metadot_mutex_trylock(metadot_mutex* _mutex) { return TryEnterCriticalSection(_mutex) ? 0 : 1; }

static inline void metadot_mutex_unlock(metadot_mutex* _mutex) { LeaveCriticalSection(_mutex); }

#elif defined(METADOT_PLATFORM_POSIX)
typedef pthread_mutex_t metadot_mutex;

static inline void metadot_mutex_init(metadot_mutex* _mutex) { pthread_mutex_init(_mutex, NULL); }

static inline void metadot_mutex_destroy(metadot_mutex* _mutex) { pthread_mutex_destroy(_mutex); }

static inline void metadot_mutex_lock(metadot_mutex* _mutex) { pthread_mutex_lock(_mutex); }

static inline int metadot_mutex_trylock(metadot_mutex* _mutex) { return pthread_mutex_trylock(_mutex); }

static inline void metadot_mutex_unlock(metadot_mutex* _mutex) { pthread_mutex_unlock(_mutex); }

#else
#error "Unsupported platform!"
#endif

class pthread_Mutex {
    metadot_mutex m_mutex;

    pthread_Mutex(const pthread_Mutex& _rhs);
    pthread_Mutex& operator=(const pthread_Mutex& _rhs);

public:
    inline pthread_Mutex() { metadot_mutex_init(&m_mutex); }
    inline ~pthread_Mutex() { metadot_mutex_destroy(&m_mutex); }
    inline void lock() { metadot_mutex_lock(&m_mutex); }
    inline void unlock() { metadot_mutex_unlock(&m_mutex); }
    inline bool tryLock() { return (metadot_mutex_trylock(&m_mutex) == 0); }
};

class ScopedMutexLocker {
    pthread_Mutex& m_mutex;

    ScopedMutexLocker();
    ScopedMutexLocker(const ScopedMutexLocker&);
    ScopedMutexLocker& operator=(const ScopedMutexLocker&);

public:
    inline ScopedMutexLocker(pthread_Mutex& _mutex) : m_mutex(_mutex) { m_mutex.lock(); }
    inline ~ScopedMutexLocker() { m_mutex.unlock(); }
};

}  // namespace MetaEngine

typedef enum engine_displaymode { WINDOWED, BORDERLESS, FULLSCREEN } engine_displaymode;

typedef enum engine_windowflashaction { START, START_COUNT, START_UNTIL_FG, STOP } engine_windowflashaction;

typedef struct engine_platform {

} engine_platform;

#define RUNNER_EXIT 2

int ParseRunArgs(int argc, char* argv[]);
int metadot_initwindow();
void metadot_endwindow();
void metadot_set_displaymode(engine_displaymode mode);
void metadot_set_windowflash(engine_windowflashaction action, int count, int period);
void metadot_set_VSync(bool vsync);
void metadot_set_minimize_onlostfocus(bool minimize);
void metadot_set_windowtitle(const char* title);
char* metadot_clipboard_get();
METAENGINE_Result metadot_clipboard_set(const char* string);

#endif  // METADOT_PLATFORM_H
