#include "buf/ProtoReader.h"
#include "buf/Header.capn.h"

#include <iostream>
#include <vector>
#include <fcntl.h>
#include <kj/io.h>
#include <kj/array.h>
#include <capnp/serialize-packed.h>
#include <buf/PackedEvent.capn.h>

#include <setup/Setup.h>
#include <match/output.h>
#include <output/GenericProvider.h>
#include <match/analyzer/CalibratedAnalyzer.h>

using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace std;

namespace {
    void read(uint8_t out, CalibratedOutput &output, Data::Reader reader) {
        output.setEnergy(out, reader.getEnergy());
        output.setTime(out, reader.getTime());
        output.setSegment(out, reader.getStrip());
    }
}

void ::AUSA::protobuf::test(std::string path, shared_ptr<Setup> setup) {

    SetupOutput output;
    const auto dCount = setup ->dssdCount();
    const auto sCount = setup ->singleCount();
    for (size_t i = 0; i < dCount; i++) {
        auto d = setup ->getDSSD(i);
        CalibratedOutput front(d, d->frontStripCount());
        CalibratedOutput back(d, d->backStripCount());
        output.addDssdOutput(CalibratedDoubleOutput(d, front, back));
    }


    for (size_t i = 0; i < sCount; i++) {
        auto d = setup ->getSingleSided(i);
        output.addSingleOutput(CalibratedSingleOutput(d, d -> segmentCount()));
    }

    Output::GenericProvider<CalibratedAnalyzer> provider;

    int fd = open(path.c_str(), O_RDONLY);

    kj::FdInputStream fdStream(fd);
    kj::BufferedInputStreamWrapper bufferedStream(fdStream);

    if (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::PackedMessageReader message(bufferedStream);

        auto header = message.getRoot<Header>();

//        auto doubles = header.getDoubles();
    }


    capnp::ReaderOptions op;
    vector<capnp::word> scratch;
    kj::ArrayPtr<capnp::word> ptr(scratch.data(), scratch.data()+scratch.size());

    while (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::PackedMessageReader message(bufferedStream, op, ptr);

        auto evt = message.getRoot<PackedEvent>();
        auto mul = evt.getMul();
        auto data = evt.getData();
        size_t count = 0;

        for (size_t i = 0; i < dCount; i++) {
            const auto m = mul[i];
            auto& out = output.getDssdOutput(i);
            auto& f = out.front();
            auto& b = out.back();

            f.setMultiplicity(m);
            b.setMultiplicity(m);
            for (uint8_t j = 0; j < m; j++) {
                read(j, f, data[count]);
                read(j, b, data[count+1]);
            }
            count+=2;
        }

        for (size_t i = 0; i < sCount; i++) {
            const auto m = mul[i+dCount];
            auto&f = output.getSingleOutput(i);

            f.setMultiplicity(m);

            for (size_t j = 0; j < m; j++) {
                read(j, f, data[count]);

                count++;
            }
        }
        provider.notifyAnalyze();
    }
}
