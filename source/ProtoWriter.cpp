#include "buf/ProtoWriter.h"
#include "buf/EventBuilder.h"
#include "buf/HeaderBuilder.h"

#include <fcntl.h>

#include <iostream>
using namespace std;

using namespace AUSA::Match;
using namespace AUSA::protobuf;
using namespace capnp;

ProtoWriter::ProtoWriter(std::string path) :
        fd(open(path.c_str(), O_RDWR|O_CREAT, 0664)) {

    fdStream = new kj::FdOutputStream(fd);

    auto N = 20;
    byte* buffer = new byte[4096*N];
    kj::ArrayPtr<byte> p(buffer, 4096*N);
    bufferedStream = new kj::BufferedOutputStreamWrapper(*fdStream, p);

    cout << bufferedStream->getWriteBuffer().size() << endl;
}

ProtoWriter::~ProtoWriter() {
    try {
        if (bufferedStream != nullptr) delete bufferedStream;
        if (fdStream != nullptr) delete fdStream;
    }
    catch (...) {

    }
}

void ProtoWriter::setup(const CalibratedSetupOutput &output) {
    AbstractCalibratedAnalyzer::setup(output);

    MallocMessageBuilder builder;
    buildHeader(builder, output);
    writePackedMessage(*bufferedStream, builder);
}

void ProtoWriter::terminate() {
    bufferedStream ->flush();
    close(fd);
}

int writer_count = 0;

void ProtoWriter::analyze() {
//    if (writer_count++ > 100) {exit(1);};

    MallocMessageBuilder builder;
    buildEvent(builder, output);
    writePackedMessage(*bufferedStream, builder);
}



