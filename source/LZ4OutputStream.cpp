#include "ausa/buf/LZ4OutputStream.h"
#include "ausa/buf/AUSALZ4.h"

#include <lz4.h>
#include <lz4hc.h>
#include <xxhash.h>
#include <ausa/AUSA.h>

using namespace std;

using namespace AUSA::buf;
using namespace AUSA::buf::LZ4;
using namespace kj;

namespace {
    void writeInt(byte* ptr, size_t number) {
        *((unsigned*) ptr) = static_cast<unsigned>(number);
    }

    template <class T>
    void writeT(byte* ptr, T t) {
        *((T*) ptr) = t;
    }
}

struct LZ4OutputStream::LZ4Wrapper {
private:
    LZ4CompressionLevel level;

    typedef std::unique_ptr<LZ4_stream_t, std::function<int(LZ4_stream_t*)>> StreamPointer;
    typedef std::unique_ptr<LZ4_streamHC_t, std::function<int(LZ4_streamHC_t*)>> HCStreamPointer;

    StreamPointer fastState;
    HCStreamPointer hcState;
    std::function<int(const char*, char*, int)> compressor;

public:
    LZ4Wrapper(LZ4CompressionLevel level) : level(level) {
        if (level == LZ4CompressionLevel::HIGH_COMPRESSION) {
            hcState = HCStreamPointer(LZ4_createStreamHC(), LZ4_freeStreamHC);
            compressor = [&](const char* src, char* dst, int size) {
                return LZ4_compressHC_continue(hcState.get(), src, dst, size);
            };
        }
        else {
            fastState = StreamPointer(LZ4_createStream(), LZ4_freeStream);
            compressor = [&](const char* src, char* dst, int size) {
                return LZ4_compress_continue(fastState.get(), src, dst, size);
            };
        }
    }

    int compress(const char* src, char* dest, int size) {
        return compressor(src, dest, size);
    }
};

LZ4OutputStream::LZ4OutputStream(OutputStream &inner, LZ4CompressionLevel compressionLevel, size_t chunkSize) : BUFFER_SIZE(chunkSize), inner(inner) {
    auto OUTPUT_SIZE = LZ4_COMPRESSBOUND(BUFFER_SIZE) + FRAME_HEADER_SIZE;

    writeBuffer[0] = heapArray<byte>(BUFFER_SIZE);
    writeBuffer[1] = heapArray<byte>(BUFFER_SIZE);
    outputBuffer = heapArray<byte>(OUTPUT_SIZE);

    activeBuffer = writeBuffer[0];
    bufferPos = activeBuffer.begin();

    // Write header
    writeInt(bufferPos + MAGIC_WORD_OFFSET, MAGIC_WORD);
    writeInt(bufferPos + BUFFER_OFFSET,     BUFFER_SIZE);
    inner.write(bufferPos, HEADER_SIZE);

    state = std::make_unique<LZ4Wrapper>(compressionLevel);
}


void LZ4OutputStream::write(const void *src, size_t size) {
    // If ever trouble. This is taken more or less from BufferedOutputStreamWrapper from capn proto

    if (KJ_LIKELY(src == bufferPos)) {
        // Oh goody, the caller wrote directly into our buffer.
        bufferPos += size;
    }
    else { // MessageWriter using it's internal slow buffer.
        size_t available = activeBuffer.end() - bufferPos;
        if (size <= available) {
            memcpy(bufferPos, src, size);
            bufferPos += size;
        } else if (size <= activeBuffer.size()) {
            // Too much for this buffer, but not a full buffer's worth, so we'll go ahead and copy.
            memcpy(bufferPos, src, available);

            compressAndWrite();

            size -= available;
            src = reinterpret_cast<const byte*>(src) + available;
            memcpy(activeBuffer.begin(), src, size);
            bufferPos = activeBuffer.begin() + size;
        } else {

            // Fill current buffer and write out.
            memcpy(bufferPos, src, available);
            compressAndWrite();

            size-=available;
            src = reinterpret_cast<const byte*>(src) + available;

            // Write out big array in chunks of size BUFFER_SIZE
            for (size_t i = 0, N = size/activeBuffer.size(); i < N; i++) {
                compressAndWrite(src, BUFFER_SIZE);
                src = reinterpret_cast<const byte*>(src) + BUFFER_SIZE;
                size-=activeBuffer.size();
            }

            // Put the rest in the buffer.
            bufferPos = activeBuffer.begin();
            write(src, size);
        }
    }
}

kj::ArrayPtr<kj::byte> LZ4OutputStream::getWriteBuffer() {
    return arrayPtr(bufferPos, activeBuffer.end());
}

void LZ4OutputStream::flush() {
    if (bufferPos > activeBuffer.begin()) {
        compressAndWrite(activeBuffer.begin(), bufferPos - activeBuffer.begin());
        bufferPos = activeBuffer.begin();
    }
}

void LZ4OutputStream::compressAndWrite(const void *src, size_t size) {
    char* destination = reinterpret_cast<char*>(outputBuffer.begin() + FRAME_HEADER_SIZE);

    auto compressedSize = state ->compress(static_cast<const char*>(src), destination, static_cast<int>(size));

    uint64_t hash = XXH64(src, size, 0);
    writeInt(outputBuffer.begin() + FRAME_SIZE_OFFSET, compressedSize);
    writeT<uint64_t>(outputBuffer.begin() + FRAME_HASH_OFFSET,   hash);

    inner.write(outputBuffer.begin(), compressedSize+FRAME_HEADER_SIZE);

    // Switch buffer
    activeBuffer = (activeBuffer == writeBuffer[0]) ? writeBuffer[1] : writeBuffer[0];
}

void LZ4OutputStream::compressAndWrite() {
    compressAndWrite(activeBuffer.begin(), BUFFER_SIZE);
}

LZ4OutputStream::~LZ4OutputStream() {
    unwindDetector.catchExceptionsIfUnwinding([&]() {
        flush();
    });
}



