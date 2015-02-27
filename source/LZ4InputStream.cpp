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
    auto writeBuffer = heapArray<byte>(100);
    auto out = inner.read(writeBuffer.begin(), 1, writeBuffer.size());

    cout << out << endl;
    cout << readInt(writeBuffer.begin()) << endl;
    unsigned int bufferSize = readInt(writeBuffer.begin()+4);
    unsigned int decomSize =
    cout << bufferSize << endl;

    stream = LZ4_createStreamDecode();

    heapArray()
}

size_t LZ4InputStream::tryRead(void *buffer, size_t minBytes, size_t maxBytes) {
    return 0;
}

kj::ArrayPtr<kj::byte const> LZ4InputStream::tryGetReadBuffer() {

}


