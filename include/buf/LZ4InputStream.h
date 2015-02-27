#ifndef BUF_LZ4_INPUT_STREAM_H
#define BUF_LZ4_INPUT_STREAM_H

#include <kj/io.h>

namespace AUSA {
    namespace protobuf {
        class LZ4InputStream : public kj::BufferedInputStream {
        public:
            LZ4InputStream(InputStream& inner);

            virtual size_t tryRead(void *buffer, size_t minBytes, size_t maxBytes);

            virtual kj::ArrayPtr<kj::byte const> tryGetReadBuffer();

        private:
            InputStream& inner;
        };
    }
}

#endif