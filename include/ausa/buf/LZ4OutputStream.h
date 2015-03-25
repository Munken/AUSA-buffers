#ifndef BUF_LZ4_OUTPUT_STREAM_H
#define BUF_LZ4_OUTPUT_STREAM_H

#include <kj/io.h>
#include <memory>


namespace AUSA {
    namespace buf {
        enum class LZ4CompressionLevel {
            DEFAULT, FAST, HIGH_COMPRESSION
        };

        /**
        * Implementation of a kj::BufferedOutputStream that compresses the output using <a href="https://code.google.com/p/lz4/">LZ4</a>.
        */
        class LZ4OutputStream : public kj::BufferedOutputStream {
        public:

            /**
            * Construct an output stream that will be LZ4 compressed on the fly.
            * With the default parameters it will use the fast LZ4 compression and a chunk size of 8 MB.
            */
            explicit LZ4OutputStream(kj::OutputStream &inner, LZ4CompressionLevel compressionLevel = LZ4CompressionLevel::DEFAULT , size_t chunkSize = 20 << 20 /*8 MB*/);

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
            class LZ4Wrapper;
            std::unique_ptr<LZ4Wrapper> state;

            const size_t BUFFER_SIZE;

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