//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <pthread.h>

namespace aphotic_shield {

class Mutex {
public:
    constexpr Mutex() = default;
    ~Mutex() = default;
    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;

    // Lock the mutex.
    void lock() {
        int Status = pthread_mutex_lock(&Mu);
        assert(Status == 0);
        (void)Status;
    }

    // Nonblocking trylock of the mutex. Returns true if the lock was acquired.
    bool tryLock() { return pthread_mutex_trylock(&Mu) == 0; }

    // Unlock the mutex.
    void unlock() {
        int Status = pthread_mutex_unlock(&Mu);
        assert(Status == 0);
        (void)Status;
    }

private:
    pthread_mutex_t Mu = PTHREAD_MUTEX_INITIALIZER;
};

class ScopedLock {
public:
    explicit ScopedLock(Mutex &Mx): Mu(Mx) { Mu.lock(); }
    ~ScopedLock() { Mu.unlock(); }
    ScopedLock(const ScopedLock &) = delete;
    ScopedLock &operator=(const ScopedLock &) = delete;

private:
    Mutex &Mu;
};

}  // namespace aphotic_shield
