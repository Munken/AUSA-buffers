#include "buf/ProtoWriter.h"
#include "buf/EventBuilder.h"
#include "buf/HeaderBuilder.h"

#include <fcntl.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>


using namespace AUSA::Match;
using namespace AUSA::protobuf;
using namespace capnp;

ProtoWriter::ProtoWriter(std::string path) {
    fd = open(path.c_str(), O_RDWR|O_CREAT, 0777);
}

void ProtoWriter::setup(const CalibratedSetupOutput &output) {
    AbstractCalibratedAnalyzer::setup(output);

    MallocMessageBuilder builder;
    buildHeader(builder, output);
    writePackedMessageToFd(fd, builder);
}

void ProtoWriter::terminate() {
    close(fd);
}

int writer_count = 0;

void ProtoWriter::analyze() {
//    if (writer_count++ > 100) {exit(1);};

    MallocMessageBuilder builder;
    buildEvent(builder, output);
    writePackedMessageToFd(fd, builder);
}


