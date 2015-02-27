#ifndef BUF_LZ4_OUTPUT_STREAM_H
#define BUF_LZ4_OUTPUT_STREAM_H

#include <kj/io.h>
#include <lz4frame.h>


namespace AUSA {
    namespace protobuf {
        class LZ4OutputStream : public kj::BufferedOutputStream {
        public:
            explicit LZ4OutputStream(kj::OutputStream& inner);
            ~LZ4OutputStream();

            void flush();

            virtual void write(const void *src, size_t size);

            virtual kj::ArrayPtr<kj::byte> getWriteBuffer();

        private:
            LZ4F_preferences_t preferences;
//            LZ4F_compressionContext_t* context;


            const size_t BUFFER_SIZE;
            size_t OUTPUT_SIZE;

            kj::ArrayPtr<kj::byte> writeBuffer, outputBuffer;
            kj::byte* bufferPos;

            kj::OutputStream& inner;
        };
    }
}

#endif