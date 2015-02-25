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
#include <AUSA.h>
#include <match/analyzer/SegmentSpectrumPlotter.h>

using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace std;

namespace {
    void read(uint8_t out, CalibratedOutput &output, Data::Reader reader) {
        double energy = reader.getEnergy();
        uint16_t t = reader.getTime();
        uint8_t segment = reader.getStrip();

//        cout << output.detector().getName() << "\t" << energy << "\t" << to_string(t) << "\t" << to_string(segment) << endl;

        output.setEnergy(out, energy);
        output.setTime(out, t);
        output.setSegment(out, segment);
    }
}

void ::AUSA::protobuf::test(std::string path, shared_ptr<Setup> setup) {

    SetupOutput output;
    const auto dCount = setup ->dssdCount();
    const auto sCount = setup ->singleCount();
    const auto sigCount = setup ->signalCount();
    for (size_t i = 0; i < dCount; i++) {
        auto d = setup ->getDSSD(i);
        CalibratedOutput front(d, d->frontStripCount());
        CalibratedOutput back(d, d->backStripCount());
        output.addDssdOutput(CalibratedDoubleOutput(d, front, back));
    }


    for (size_t i = 0; i < sCount; i++) {
        auto d = setup ->getSingleSided(i);
        CalibratedSingleOutput singleOutput = CalibratedSingleOutput(d, d -> segmentCount());
        singleOutput.setMultiplicity(0);
        output.addSingleOutput(singleOutput);
    }

    for (int i = 0; i < sigCount; i++) {
        output.addScalerOutput(CalibratedSignal{make_unique<UInt_t>(), setup ->getScaler().getChannelName(i)});
    }

    Output::GenericProvider<CalibratedAnalyzer> provider;

    provider.attach(make_shared<SegmentSpectrumPlotter>(0, 5000));

    int fd = open(path.c_str(), O_RDONLY);

    auto N = 50;
    capnp::byte* buffer = new capnp::byte[4096*N];
    kj::ArrayPtr<capnp::byte> p(buffer, 4096*N);

    kj::FdInputStream fdStream(fd);
    kj::BufferedInputStreamWrapper bufferedStream(fdStream, p);

    if (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::PackedMessageReader message(bufferedStream);

        auto header = message.getRoot<Header>();

//        auto doubles = header.getDoubles();
//        for (auto d : doubles) {
//            cout << d.getName(). << " (" << d.getFrontStrips() << ", " << d.getBackStrips() << endl;
//        }
    }


    provider.notifySetup(output);


    capnp::ReaderOptions op;
    vector<capnp::word> scratch;
    kj::ArrayPtr<capnp::word> ptr(scratch.data(), scratch.data()+scratch.size());

    int t = 0;
    while (bufferedStream.tryGetReadBuffer() != nullptr && t<100) {
        ::capnp::PackedMessageReader message(bufferedStream, op, ptr);

        auto evt = message.getRoot<PackedEvent>();
        auto mul = evt.getMul();
        auto data = evt.getData();
        auto sig = evt.getSignal();
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
//                cout << endl;
                count+=2;
            }
        }

        for (size_t i = 0; i < sCount; i++) {
            const auto m = mul[i+dCount];
            auto& f = output.getSingleOutput(i);

            f.setMultiplicity(m);

            for (size_t j = 0; j < m; j++) {
                Data::Reader reader = data[count];
                double energy = reader.getEnergy();
                uint16_t t = reader.getTime();
                uint8_t segment = reader.getStrip();
                read(j, f, reader);

                count++;
            }
        }

        for (size_t i = 0; i < sigCount; i++) {
            output.getScalerOutput(i).setValue(sig[i]);
        }
        provider.notifyAnalyze();
    }
    provider.notifyTerminate();
    provider.notifySaveRoot("cap.root", "RECREATE");
}
