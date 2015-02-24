#include "buf/ProtoReader.h"
#include "buf/Header.capn.h"

#include <iostream>
#include <vector>
#include <fcntl.h>
#include <kj/io.h>
#include <kj/array.h>
#include <capnp/serialize-packed.h>
#include <buf/Event.capn.h>

using namespace AUSA::protobuf;
using namespace std;

void ::AUSA::protobuf::test(std::string path) {
    int fd = open(path.c_str(), O_RDONLY);

    kj::FdInputStream fdStream(fd);
    kj::BufferedInputStreamWrapper bufferedStream(fdStream);

    if (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::PackedMessageReader message(bufferedStream);

        auto header = message.getRoot<Header>();

//        auto doubles = header.getDoubles();
    }

    size_t i = 0;

    capnp::ReaderOptions op;
    vector<capnp::word> scratch;
    kj::ArrayPtr<capnp::word> ptr(scratch.data(), scratch.data()+scratch.size());

    while (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::PackedMessageReader message(bufferedStream, op, ptr);

        auto evt = message.getRoot<Event>();
        i++;
    }
    cout << i << endl;
}
