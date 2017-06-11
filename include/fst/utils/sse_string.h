#pragma once

#include <iostream>
#include <vector>
#include <fst/buffer_view.h>
#include <xmmintrin.h>
///<xmmintrin.h> SSE
///<emmintrin.h> SSE2
///<pmmintrin.h> SSE3
///<tmmintrin.h> SSSE3

namespace fst {
namespace sse {
    template <char cvalue> int strchr(fst::buffer_view<char> data)
    {
        __m128i cx16 = _mm_set1_epi8(cvalue); // cvalue replicated 16 times.

        for (int i = 0; i < data.size(); i += 16) {
            __m128i x = _mm_loadu_si128((__m128i const*)&data[i]);
            unsigned v = _mm_movemask_epi8(_mm_cmpeq_epi8(cx16, x));

            if (v) {
                return i + ffs(v) - 1;
            }
        }

        return -1;
    }

    template <char cvalue> void find_all_char(fst::buffer_view<char> data, std::vector<std::size_t>& indexes)
    {
        const __m128i cx16 = _mm_set1_epi8(cvalue); // cvalue replicated 16 times.
        const char* c_begin = data.data();

        if (((intptr_t)c_begin & 0xF) != 0) {
            const __m128i x = _mm_loadu_si128((__m128i const*)c_begin);
            unsigned int v = _mm_movemask_epi8(_mm_cmpeq_epi8(cx16, x));
            int pos;
            while ((pos = ffs(v))) {
                indexes.push_back(pos - 1);
                v &= ~(1 << (pos - 1));
            }
        }

        const char* c_ptr = (const char*)(0x10 + (intptr_t)c_begin & ~0xF);

        for (long i = c_ptr - c_begin; i < data.size(); i += 16) {
            const __m128i x = _mm_load_si128((__m128i const*)&data[i]);
            unsigned int v = _mm_movemask_epi8(_mm_cmpeq_epi8(cx16, x));

            int pos;
            while ((pos = ffs(v))) {
                indexes.push_back(i + pos - 1);
                v &= ~(1 << (pos - 1));
            }
        }
    }

    inline int popcount(int v)
    {
        v = v - ((v >> 1) & 0x55555555); // put count of each 2 bits into those 2 bits
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333); // put count of each 4 bits into those 4 bits
        return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
    }

    inline std::size_t count_char(fst::buffer_view<char> data, char cvalue)
    {
        std::size_t count = 0;
        const __m128i cx16 = _mm_set1_epi8(cvalue); // cvalue replicated 16 times.
        for (std::size_t i = 0; i < data.size(); i += 16) {
            count += popcount(
                _mm_movemask_epi8(_mm_cmpeq_epi8(cx16, _mm_load_si128((__m128i const*)&data[i]))));
        }

        return count;
    }
} // sse
} // fst.
