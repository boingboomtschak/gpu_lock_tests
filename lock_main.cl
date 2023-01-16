
static void lock(global atomic_uint* l) {
    uint e = 0;
    uint acq = 0;
    while (acq == 0) {
        acq = atomic_compare_exchange_strong_explicit(l, &e, 1, memory_order_acquire, memory_order_relaxed);
        e = 0;
    }
}

static void unlock(global atomic_uint* l) {
    atomic_store_explicit(l, 0, memory_order_release);
}

kernel void lock_test(global atomic_uint* l, global uint* res, global uint* iters) {
    for (uint i = 0; i < iters[0]; i++) {
        lock(l);

        uint x = *res;
        x++;
        *res = x;

        unlock(l);
    }
}
