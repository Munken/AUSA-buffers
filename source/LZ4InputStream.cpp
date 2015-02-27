#include <glob.h>
#include <kj/common.h>
#include <lz4frame.h>
#include "buf/LZ4InputStream.h"

#include <iostream>

using namespace AUSA::protobuf;
using namespace kj;
using namespace std;

namespace {
    unsigned readInt(byte* ptr) {
        return *((unsigned*) ptr);
    }
}

LZ4InputStream::LZ4InputStream(InputStream &inner) : inner(inner) {
    auto writeBuffer = heapArray<byte>(8);
    auto out = inner.read(writeBuffer.begin(), 1, 8);

    cout << out << endl;
    unsigned int bufferSize = readInt(writeBuffer.begin());
    unsigned int decomSize = LZ4_COMPRESSBOUND(bufferSize);
    unsigned int frameSize = readInt(writeBuffer.begin() + 4);
    cout << bufferSize << endl;
    cout << decomSize << endl;
    cout << frameSize << endl;

    stream = LZ4_createStreamDecode();

    kj::Array<byte> compressed = heapArray<byte>(bufferSize);
    kj::Array<byte> decompressed = heapArray<byte>(decomSize);

    auto innerRet = inner.read(compressed.begin(), frameSize, frameSize);
    cout << innerRet << endl;
    auto ret = LZ4_decompress_safe_continue(stream, (char const *) compressed.begin(), (char *) decompressed.begin(), frameSize, decomSize);
    cout << ret << endl;

    for (int i = 0; i < ret; i++) cout << decompressed[i];
}

size_t LZ4InputStream::tryRead(void *buffer, size_t minBytes, size_t maxBytes) {
    return 0;
}

kj::ArrayPtr<kj::byte const> LZ4InputStream::tryGetReadBuffer() {

}


