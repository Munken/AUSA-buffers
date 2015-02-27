#ifndef BUF_LZ4_OUTPUT_STREAM_H
#define BUF_LZ4_OUTPUT_STREAM_H

#include <kj/io.h>
#include <lz4.h>


namespace AUSA {
    namespace protobuf {
        class LZ4OutputStream : public kj::BufferedOutputStream {
        public:

            /**
            * Construct an output stream that will be LZ4 compressed on the fly.
            * With the default parameters it will use the fast LZ4 compression and a chunk size of 8 MB.
            */
            explicit LZ4OutputStream(kj::OutputStream& inner, unsigned compressionLevel = 0, size_t chunkSize = 8 << 20 /*8 MB*/);

            /**
            * This will flush the output to the underlying output stream.
            * This might fail if that stream is already deleted.
            */
            ~LZ4OutputStream();

            /**
            * Flush all data in the buffer to the underlying stream.
            */
            void flush();

            /**
            * Write size elements from src into the buffer.
            * If src is the write buffer then this will just increment the buffer position.
            * However this will not check if the resulting size overflow the internal buffer!
            *
            * If the size is larger than the internal buffer then src will be written out in chunks.
            */
            virtual void write(const void *src, size_t size) override;

            /**
            * Get a pointer to the interrnal buffer.
            */
            virtual kj::ArrayPtr<kj::byte> getWriteBuffer() override;

        private:
//            LZ4F_preferences_t preferences;
            LZ4_stream_t* stream;
//
            const size_t BUFFER_SIZE;
            size_t OUTPUT_SIZE;
//
            kj::Array<kj::byte> writeBuffer[2], outputBuffer;
            kj::ArrayPtr<kj::byte> activeBuffer;
            kj::byte* bufferPos;

            kj::OutputStream& inner;

            kj::UnwindDetector unwindDetector;

            void compressAndWrite(const void* src, size_t size);
            void compressAndWrite();
        };
    }
}

#endif