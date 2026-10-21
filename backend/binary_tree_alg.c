#include "../deter.h"
#include <immintrin.h>
#include <stdint.h>

_Static_assert(RAND_MAX == 2147483647, "Bit batching requires a 31-bit rand()");
_Static_assert(sizeof(Cell) == 1, "SIMD requires one byte per cell");

typedef struct {
    uint64_t bits;
    unsigned int available;
} BinaryBits;

/* Preserve the scalar stream: low bits first, 31 bits from each rand(). */
static inline uint32_t take_bits(BinaryBits *stream, unsigned int count)
{
    while (stream->available < count) {
        stream->bits |= (uint64_t)(unsigned int)rand() << stream->available;
        stream->available += 31;
    }
    uint32_t result = (uint32_t)stream->bits;
    stream->bits >>= count;
    stream->available -= count;
    return result;
}

/* Expand 32 packed decisions to 32 bytes containing either 0x00 or 0xff. */
static inline __m256i expand_bits(uint32_t bits)
{
    const __m256i bytes = _mm256_setr_epi8(
        0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
        2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3);
    const __m256i powers = _mm256_setr_epi8(
        1,2,4,8,16,32,64,-128, 1,2,4,8,16,32,64,-128,
        1,2,4,8,16,32,64,-128, 1,2,4,8,16,32,64,-128);
    __m256i spread = _mm256_shuffle_epi8(_mm256_set1_epi32((int)bits), bytes);
    return _mm256_cmpeq_epi8(_mm256_and_si256(spread, powers), powers);
}

void binary_algos(MazeTable table)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
        return;

    /* Derive byte masks from Cell instead of assuming a bit-field layout. */
    const Cell top = {.wall = {.top = 1}};
    const Cell right = {.wall = {.right = 1}};
    const Cell bottom = {.wall = {.bottom = 1}};
    const Cell left = {.wall = {.left = 1}};
    const __m256i tm = _mm256_set1_epi8((char)top.all_bits);
    const __m256i rm = _mm256_set1_epi8((char)right.all_bits);
    const __m256i bm = _mm256_set1_epi8((char)bottom.all_bits);
    const __m256i lm = _mm256_set1_epi8((char)left.all_bits);
    BinaryBits stream = {0};

    for (int y = 0; y < table.rows - 1; y++) {
        unsigned char *row = (unsigned char *)table.data[y];
        unsigned char *below = (unsigned char *)table.data[y + 1];
        int x = 0;
        /* Virtual downward choice before column zero keeps the left boundary. */
        uint32_t previous_down = 1;
        for (; table.columns - 1 - x >= 32; x += 32) {
            uint32_t choices = take_bits(&stream, 32);
            __m256i down = expand_bits(choices);
            __m256i previous = expand_bits((choices << 1) | previous_down);
            __m256i clear = _mm256_or_si256(
                _mm256_and_si256(down, bm),
                _mm256_andnot_si256(down, rm));
            clear = _mm256_or_si256(clear, _mm256_andnot_si256(previous, lm));
            __m256i current = _mm256_loadu_si256((const __m256i *)(row + x));
            _mm256_storeu_si256((__m256i *)(row + x),
                _mm256_andnot_si256(clear, current));
            /* Update the top walls below without overlapping current-row stores. */
            __m256i next = _mm256_loadu_si256((const __m256i *)(below + x));
            _mm256_storeu_si256((__m256i *)(below + x),
                _mm256_andnot_si256(_mm256_and_si256(down, tm), next));
            previous_down = choices >> 31;
        }
        for (; x < table.columns - 1; x++) {
            unsigned int down = take_bits(&stream, 1) & 1u;
            /* Keep the scalar tail free of branches on random decisions. */
            unsigned char clear = (bottom.all_bits & (0u - down)) |
                (right.all_bits & (0u - (down ^ 1u))) |
                (left.all_bits & (0u - (previous_down ^ 1u)));
            row[x] &= (unsigned char)~clear;
            below[x] &= (unsigned char)~(top.all_bits & (0u - down));
            previous_down = down;
        }
        /* Last column opens downward without consuming a random bit. */
        unsigned char clear = bottom.all_bits;
        if (!previous_down)
            clear |= left.all_bits;
        row[x] &= (unsigned char)~clear;
        below[x] &= (unsigned char)~top.all_bits;
    }
    /* Last row opens rightward, preserving top walls from the previous row. */
    if (table.columns > 1) {
        unsigned char *row = (unsigned char *)table.data[table.rows - 1];
        row[0] &= (unsigned char)~right.all_bits;
        int x = 1;
        const __m256i clear = _mm256_or_si256(lm, rm);
        for (; table.columns - 1 - x >= 32; x += 32) {
            __m256i current = _mm256_loadu_si256((const __m256i *)(row + x));
            _mm256_storeu_si256((__m256i *)(row + x),
                _mm256_andnot_si256(clear, current));
        }
        for (; x < table.columns - 1; x++)
            row[x] &= (unsigned char)~(left.all_bits | right.all_bits);
        row[x] &= (unsigned char)~left.all_bits;
    }
    labirit_metrics_step((long long)table.rows * table.columns);
}
