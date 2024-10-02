/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "mm.h"
#include "spinlock.h"
#include "cuda_helper.h"
#include "u64_bitmap.h"
#include "print_util.h"

/*
 *   Memory pool consists of 64 mem_partitions, each with an array of bitmap indexes to track free slots.
 *   Each bitmap index is a two-level 64-bit bitmap, with the root bitmap indicating which leaf has free slots,
 *   and each leaf bitmap indicating which slot is free.
 *
 *                                   root bitmap
 *        |       |       |       |       |       |       |       |       |
 *       leaf    leaf    leaf    leaf    leaf    leaf    leaf    leaf    leaf  ...
 *     |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
 *   slot slot slot slot slot slot slot .... slot slot slot slot slot slot slot  ...
 *
 *   Each slot can be 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, or 256 bytes.
 *   64 GB memory address space is reserved for each mem_partition, and 1 TB memory address space is reserved for the whole memory pool.
 *   1 GB memory address space is reserved for each array of bitmap indexes, and 16 GB memory address space is reserved for the whole memory pool.
 */

#define MEM_SLOT_SIZE_COUNT 16

#define NUM_MEM_POOLS 16

static u64 const MEM_PARTITION_ADDR_SPACE = 0x1000000000; // 64 GB == (1 << 36) bytes
// 1 TB (64 GB X 16) memory address space will be reserved for memory pool
static u64 const MEM_POOL_ADDR_SPACE = 0x10000000000; // 1 TB

static u64 const INDEX_PARTITION_ADDR_SPACE = 0x40000000; // 1 GB == (1 << 30) bytes
static u64 const INDEX_ADDR_SPACE = 0x400000000; // 16 GB == (1 << 34) bytes

struct mem_index_block
{
    u64 root_bitmap; // root is a bitmap of 64 bits, indicating which leaf has free slots
    u64 leaf_bitmaps[64]; // each leaf is a bitmap of 64 bits, indicating which slot is free
};

struct mem_pool
{
    void* mem;
    void* indexes;
    u64 index_counts[MEM_SLOT_SIZE_COUNT];
};

static atomic_u64 total_allocated_memory = 0;
static atomic_u64 total_allocated_pooled_memory = 0;
static atomic_u64 total_committed_memory_requested = 0;
static atomic_u64 total_decommited_memory_requested = 0;

static atomic_u64 malloc_count = 0;
static atomic_u64 calloc_count = 0;
static atomic_u64 free_count = 0;

static atomic_u64 pooled_malloc_count = 0;
static atomic_u64 pooled_calloc_count = 0;
static atomic_u64 pooled_free_count = 0;

//static volatile atomic_bool mm_memory_pool_locks[NUM_MEM_POOLS];

static struct mem_pool* mem_pools = NULL;
//static volatile atomic_bool* mem_index_block_locks;
static u64 page_size = 0;
volatile atomic_bool* index_counts_locks;
thread_local u64 mm_thread_id = U64_MAX;

static inline void increase_malloc_count(u64 count, u64 size)
{
    malloc_count += count;
    total_allocated_memory += size;
}

static inline void increase_calloc_count(u64 count, u64 size)
{
    calloc_count += count;
    total_allocated_memory += size;
}

static inline void increase_free_count(u64 count)
{
    free_count += count;
}

static inline void increase_pooled_malloc_count(u64 count, u64 size)
{
    pooled_malloc_count += count;
    total_allocated_pooled_memory += size;
}

static inline void increase_pooled_calloc_count(u64 count, u64 size)
{
    pooled_calloc_count += count;
    total_allocated_pooled_memory += size;
}

static inline void increase_pooled_free_count(u64 count)
{
    pooled_free_count += count;
}

static inline void* reserve_memory_space(u64 size)
{
    void* ptr = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return MAP_FAILED == ptr ? NULL : ptr;
}

static inline void commit_memory(void* ptr, u64 size)
{
    u64 offset_in_last_committed_page = (page_size - 1) & (u64)ptr;
    u64 last_fully_occupied_page_addr = (~(page_size - 1)) & (u64)ptr;
    if (mprotect((void*)last_fully_occupied_page_addr, offset_in_last_committed_page + size, PROT_READ | PROT_WRITE))
    {
        printf("Error: failed to commit memory.\n");
    }
    total_committed_memory_requested += size; // what total_committed_memory_requested tracks is the size requested by the user, not the memory actually committed
}

static inline u64 decommit_memory(void* ptr, u64 size)
{
    if (mprotect(ptr, size, PROT_NONE))
    {
        printf("Error: failed to call mprotect in decommit_memory.\n");
        return 0;
    }

    if (madvise(ptr, size, MADV_DONTNEED))
    {
        printf("Error: failed to call madvise in decommit_memory.\n");
        return 0;
    }

    total_decommited_memory_requested += size; // what total_decommited_memory_requested tracks is the size requested by the user, not the memory actually decommitted
    return 1;
}

static u64 release_reserved_memory_address_space(void* ptr, u64 size)
{
    if (munmap(ptr, size) != 0)
    {
        printf("Error: failed to free reserved memory address space.\n");
        return 0;
    }
    return 1;
}

void print_mem_pool(struct mem_pool* pool)
{
    printf("mem: %p\n", pool->mem);
    printf("indexes: %p\n", pool->indexes);
    for (u64 i = 0; i < MEM_SLOT_SIZE_COUNT; ++i)
    {
        printf("index_counts[%lu]: %lu\n", i, pool->index_counts[i]);
    }
}

static void mm_create_pool(struct mem_pool* pool)
{
    u64 index_block_count_per_partition = page_size / sizeof(struct mem_index_block);
    u64 index_size_per_partition = index_block_count_per_partition * sizeof(struct mem_index_block);

    for (u64 i = 0; i < MEM_SLOT_SIZE_COUNT; ++i)
    {
        pool->index_counts[i] = index_block_count_per_partition;

        u64 index_offset = i * INDEX_PARTITION_ADDR_SPACE;
        commit_memory(pool->indexes + index_offset, index_size_per_partition);
        memset(pool->indexes + index_offset, 0, index_size_per_partition);

        u64 slot_size = (i + 1) << 4; // 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256
        u64 mem_partition_size = slot_size << 12; // 64 X 64 memory slots
        u64 mem_size = mem_partition_size * index_block_count_per_partition;

        commit_memory(pool->mem + MEM_PARTITION_ADDR_SPACE * i, mem_size);
    }
}

void mm_initialize()
{
    mem_pools = calloc(NUM_MEM_POOLS, sizeof(struct mem_pool));
    increase_calloc_count(1, NUM_MEM_POOLS * sizeof(struct mem_pool));
    void* mem = reserve_memory_space(NUM_MEM_POOLS * MEM_POOL_ADDR_SPACE);
    void* indexes = reserve_memory_space(NUM_MEM_POOLS * INDEX_ADDR_SPACE);
    page_size = (u64)sysconf(_SC_PAGESIZE);

    // there are at most 64 GB / (4K * 16) == 1M mem_index_blocks per mem partition
    //u64 max_mem_index_block_count = (NUM_MEM_POOLS * MEM_SLOT_SIZE_COUNT) << 20;
    //mem_index_block_locks = malloc(max_mem_index_block_count * sizeof(atomic_bool));
    //increase_malloc_count(1, max_mem_index_block_count * sizeof(atomic_bool));

    index_counts_locks = calloc(MEM_SLOT_SIZE_COUNT * NUM_MEM_POOLS, sizeof(atomic_bool));
    increase_calloc_count(1, MEM_SLOT_SIZE_COUNT * NUM_MEM_POOLS * sizeof(atomic_bool));

    for (u64 i = 0; i < NUM_MEM_POOLS; ++i)
    {
        struct mem_pool* pool = mem_pools + i;
        pool->mem = mem + i * MEM_POOL_ADDR_SPACE;
        pool->indexes = indexes + i * INDEX_ADDR_SPACE;
        mm_create_pool(pool);
    }
}

static void mm_free_pool(struct mem_pool* pool)
{
    for (u64 i = 0; i < 16; ++i)
    {
        u64 slot_size = (i + 1) << 4;
        u64 mem_partition_size = slot_size << 12; // 64 X 64 memory slots

        decommit_memory(pool->mem + i * MEM_PARTITION_ADDR_SPACE, mem_partition_size * pool->index_counts[i]);
        decommit_memory(pool->indexes + i * INDEX_PARTITION_ADDR_SPACE, sizeof(struct mem_index_block) * pool->index_counts[i]);
    }
}

void mm_finalize()
{
    for (u64 i = 0; i < NUM_MEM_POOLS; ++i)
    {
        mm_free_pool(mem_pools + i);
    }

    release_reserved_memory_address_space(mem_pools->mem, NUM_MEM_POOLS * MEM_POOL_ADDR_SPACE);
    release_reserved_memory_address_space(mem_pools->indexes, NUM_MEM_POOLS * INDEX_ADDR_SPACE);

    free(mem_pools);
    //free(mem_index_block_locks);
    free((void*)index_counts_locks);
    increase_free_count(2);
}

static void* pooled_malloc(u64 size)
{
    u64 slot_size = (size + 15) & ~0xF;
    u64 slot_size_id = (slot_size >> 4) - 1;
    u64 mem_partition_size = slot_size << 12; // 64 X 64 memory slots

    u64 pool_id = mm_thread_id;
    if (pool_id >= NUM_MEM_POOLS)
    {
        pool_id = (malloc_count + calloc_count) & 0xF; // equivalent to (malloc_count + calloc_count) % NUM_MEM_POOLS
    }

    volatile atomic_bool* lck = spinlock_lock_one(index_counts_locks + slot_size_id * NUM_MEM_POOLS, NUM_MEM_POOLS, &pool_id);
    struct mem_pool* pool = mem_pools + pool_id;
    void* mem = pool->mem + slot_size_id * MEM_PARTITION_ADDR_SPACE;
    struct mem_index_block* index = pool->indexes + slot_size_id * INDEX_PARTITION_ADDR_SPACE;

    u64 index_count = pool->index_counts[slot_size_id];
    for (u64 i = 0; i < index_count; ++i)
    {
        if (index->root_bitmap == U64_MAX)
        {
            index++;
            mem += mem_partition_size;
            continue;
        }

        u64 leaf_idx = u64_bitmap_first_unset_bit(index->root_bitmap);
        u64* leaf = &(index->leaf_bitmaps[leaf_idx]);
        u64 slot_idx_in_leaf = u64_bitmap_first_unset_bit(*leaf);
        u64 slot_idx_in_block = (leaf_idx << 6) + slot_idx_in_leaf;

        u64_bitmap_set(leaf, slot_idx_in_leaf);
        if (*leaf == U64_MAX)
        {
            u64_bitmap_set(&(index->root_bitmap), leaf_idx);
        }

        spinlock_unlock(lck);
        return mem + slot_idx_in_block * slot_size;
    }

    commit_memory(mem, mem_partition_size);
    commit_memory(index, sizeof(struct mem_index_block));
    memset(index, 0, sizeof(struct mem_index_block));
    pool->index_counts[slot_size_id] += 1;

    u64* leaf = index->leaf_bitmaps;
    u64_bitmap_set(leaf, 0);

    spinlock_unlock(lck);
    return mem;
}

static u64 pooled_free(void* ptr)
{
    u64 pool_id = (ptr - mem_pools->mem) >> 40; // equivalent to ptr_offset / MEM_POOL_ADDR_SPACE == ptr_offset / 1 TB
    struct mem_pool* pool = mem_pools + pool_id;

    u64 mem_offset_in_pool = ptr - pool->mem;
    u64 slot_size_id = mem_offset_in_pool >> 36; // equivalent to ptr_offset / MEM_PARTITION_ADDR_SPACE == ptr_offset / 64 GB
    u64 slot_size = (slot_size_id + 1) << 4; // 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256
    u64 mem_offset_in_partition = mem_offset_in_pool - (slot_size_id << 36);
    u64 mem_block_size = slot_size << 12; // 64 X 64 memory slots
    u64 index_block_id = mem_offset_in_partition / mem_block_size;
    u64 slot_index_in_block = (mem_offset_in_partition - index_block_id * mem_block_size) / slot_size;
    u64 leaf_index = slot_index_in_block >> 6; // equivalent to slot_idx_in_array / 64

    struct mem_index_block* index = pool->indexes + slot_size_id * INDEX_PARTITION_ADDR_SPACE + index_block_id * sizeof(struct mem_index_block);
    u64 slot_index_in_leaf = slot_index_in_block & 0x3F; // equivalent to slot_index_in_block % 64

    u64* leaf = &(index->leaf_bitmaps[leaf_index]);
    if (slot_index_in_leaf >= 64)
    {
        printf("Error: slot_index_in_leaf >= 64.\n");
        return 0;
    }

    volatile atomic_bool* lck = index_counts_locks + slot_size_id * NUM_MEM_POOLS + pool_id;
    spinlock_lock(lck);
    u64_bitmap_unset(leaf, slot_index_in_leaf);
    if (*leaf == 0)
    {
        u64_bitmap_unset(&(index->root_bitmap), leaf_index);
    }
    spinlock_unlock(lck);
    return 1;
}

void* x_malloc_impl(u64 size)
{
#ifdef USE_CUDA
    void* ptr;
    cudaMallocManaged(&ptr, size, cudaMemAttachGlobal);
    increase_malloc_count(1, size);
    return ptr;
#else
    if (size > 256 || size == 0)
    {
        increase_malloc_count(1, size);
        return malloc(size);
    }
    else
    {
        void* ptr = pooled_malloc(size);
        increase_pooled_malloc_count(1, size);
        return ptr;
    }
#endif
}

void* x_calloc_impl(u64 count, u64 size)
{
    u64 _size = count * size;
#ifdef USE_CUDA
    void* ptr;
    cudaMallocManaged(&ptr, _size, cudaMemAttachGlobal);
    increase_calloc_count(1, _size);
    memset(ptr, 0, _size);
    return ptr;
#else
    if (_size > 256 || _size == 0)
    {
        increase_calloc_count(1, _size);
        return calloc(count, size);
    }
    else
    {
        void* ptr = pooled_malloc(_size);
        memset(ptr, 0, _size);
        increase_pooled_calloc_count(1, _size);
        return ptr;
    }
#endif
}

void x_free_impl(void* ptr)
{
    if (ptr == NULL)
    {
        printf("Warning: NULL pointer is being freed.\n");
    }

#ifdef USE_CUDA
    increase_free_count(1);
    cudaFree(ptr);
#else
    if (ptr < mem_pools->mem || ptr >= (mem_pools->mem + (MEM_POOL_ADDR_SPACE << 4)))
    {
        free(ptr);
        increase_free_count(1);
    }
    else
    {
        pooled_free(ptr);
        increase_pooled_free_count(1);
    }
#endif
}

static void print_memory_size(u64 size)
{
    if (size < 1024)
    {
        printf("%lu bytes\n", size);
    }
    else if (size < 1024 * 1024)
    {
        printf("%lu bytes (%.2f KB)\n", size, (f64)size / 1024.0);
    }
    else if (size < 1024 * 1024 * 1024)
    {
        printf("%lu bytes (%.2f MB)\n", size, (f64)size / (1024.0 * 1024.0));
    }
    else
    {
        printf("%lu bytes (%.2f GB)\n", size, (f64)size / (1024.0 * 1024.0 * 1024.0));
    }
}

void mm_print_status()
{
    print_block_line("Memory Management Summary");
    u64 total_alloc_times = malloc_count + calloc_count;
    printf("Alloc %lu times: ", total_alloc_times);
    print_memory_size(total_allocated_memory);
    printf("Free %lu times\n", free_count);
    if (total_alloc_times == free_count)
    {
        printf("Status: OK\n");
    }
    else
    {
        printf("Warning: alloc times and free times don't match.\n");
    }
    printf("\n");

    u64 total_pooled_alloc_times = pooled_malloc_count + pooled_calloc_count;
    printf("Pooled alloc %lu times: ", total_pooled_alloc_times);
    print_memory_size(total_allocated_pooled_memory);
    printf("Pooled free %lu times\n", pooled_free_count);
    if (total_pooled_alloc_times == pooled_free_count)
    {
        printf("Status: OK\n");
    }
    else
    {
        printf("Warning: pooled alloc times and free times don't match.\n");
    }
    printf("\n");

    printf("Total committed memory requested: ");
    print_memory_size(total_committed_memory_requested);
    printf("Total decommitted memory requested: ");
    print_memory_size(total_decommited_memory_requested);
    if (total_committed_memory_requested == total_decommited_memory_requested)
    {
        printf("Status: OK\n");
    }
    else
    {
        printf("Warning: total committed memory requested and total decommitted memory requested don't match.\n");
    }
    print_block_line(NULL);
}

#ifdef TRACE_MEMORY
void* x_malloc_trace(u64 size, char* file, u64 line)
{
    void* ptr = x_malloc_impl(size);
    fprintf(stderr, "%s:%lu x_malloc(%lu) %p\n", file, line, size, ptr);
    return ptr;
}

void* x_calloc_trace(u64 count, u64 size, char* file, u64 line)
{
    void* ptr = x_calloc_impl(count, size);
    fprintf(stderr, "%s:%lu x_calloc(%lu,%lu) %p\n", file, line, count, size, ptr);
    return ptr;
}

void x_free_trace(void* ptr, char* file, u64 line)
{
    x_free_impl(ptr);
    fprintf(stderr, "%s:%lu x_free %p\n", file, line, ptr);
}
#endif

#ifdef MODULE_TEST
#include "cli_args.h"

static u64 force_run_all_tests = 0;

static void test_mem_pool(u64 run)
{
    if (!force_run_all_tests && !run)
    {
        return;
    }

    printf("sizeof(struct mem_index_array): %lu\n", sizeof(struct mem_index_block));

    u64 count = 333;

    void* ptrs[count];

    for (u64 i = 1; i < count; ++i)
    {
        void* ptr = NULL;

        if (i < 18)
        {
            ptr = x_malloc(i % 256);
        }
        else
        {
            ptr = x_calloc(1, i % 256);
        }
        printf("ptr: %p size: %lu\n", ptr, i);

        u64* u64_ptr = (u64*)ptr;
        *u64_ptr = i;

        ptrs[i] = ptr;
    }

    for (u64 i = 1; i < count; ++i)
    {
        printf("ptrs[%lu]: %lu\n", i, *(u64*)ptrs[i]);
        x_free(ptrs[i]);
    }
}

int main(int argc, char* argv[])
{
    mm_initialize();
    force_run_all_tests = cli_check_flag(argc, argv, "--run-all");

    test_mem_pool(1);

    mm_finalize();
    mm_print_status();
    return 0;
}
#endif
