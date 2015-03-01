#ifndef BUF_LZ4_INPUT_STREAM_H
#define BUF_LZ4_INPUT_STREAM_H

#include <memory>
#include <kj/io.h>
#include "AUSALZ4.h"

namespace AUSA {
    namespace protobuf {
        class LZ4InputStream : public kj::BufferedInputStream {
        public:
            LZ4InputStream(InputStream& inner);

            virtual size_t tryRead(void *buffer, size_t minBytes, size_t maxBytes);

            virtual kj::ArrayPtr<kj::byte const> tryGetReadBuffer();

        private:
            InputStream& inner;
            kj::Array<kj::byte> compressedBuffer, decompressedBuffer;
            kj::ArrayPtr<kj::byte> bufferAvailable;

            class StreamState;
            std::unique_ptr<StreamState> state;

            size_t readCompressed();
        };
    }
}

#endif