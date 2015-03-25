#ifndef BUF_AUSA_LZ4_H
#define BUF_AUSA_LZ4_H

#include <stdint.h>
#include <stddef.h>

namespace AUSA {
    /**
    * This document outlines the layout of a AUSAbuf LZ4 compressed frame.
    * A file will consist of a starting 32 bit magic word.
    * This is followed by a 32 bit integer indicating the buffer size for the uncompressed data.
    *
    * This is then followed by a unspecified number of frames.
    *
    * A frame will consist of:
    * 32 bit integer entailing the size of the frame.
    * 64 bit hash value of the uncompressed data.
    * This amount of bits which is compressed capn proto messages.
    */
    namespace protobuf {
        namespace LZ4 {
            const uint32_t MAGIC_WORD = 0xA05A;

            typedef uint32_t BufferSize_t;
            typedef uint64_t Hash_t;

            const size_t MAGIC_WORD_SIZE = sizeof(MAGIC_WORD);
            const size_t BUFFER_WORD_SIZE = sizeof(BufferSize_t);
            const size_t HEADER_SIZE = MAGIC_WORD_SIZE + BUFFER_WORD_SIZE;

            const size_t MAGIC_WORD_OFFSET = 0;
            const size_t BUFFER_OFFSET = MAGIC_WORD_SIZE;
            const size_t FIRST_FRAME_OFFSET = HEADER_SIZE;

            const size_t FRAME_SIZE_SIZE = sizeof(BufferSize_t);
            const size_t FRAME_HASH_SIZE = sizeof(Hash_t);

            const size_t FRAME_SIZE_OFFSET = 0;
            const size_t FRAME_HASH_OFFSET = FRAME_SIZE_SIZE;
            const size_t FRAME_HEADER_SIZE = FRAME_HASH_SIZE + FRAME_SIZE_SIZE;
        }
    }
}

#endif