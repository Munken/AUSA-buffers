#include "buf/ProtoReader.h"
#include "buf/Header.capn.h"

#include <iostream>
#include <fcntl.h>
#include <capnp/serialize-packed.h>
#include <buf/PackedEvent.capn.h>

#include <AUSA.h>
#include <buf/LZ4InputStream.h>

using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace AUSA;
using namespace std;

namespace {
    void read(uint8_t out, CalibratedOutput &output, Data::Reader reader) {
        double energy = reader.getEnergy();
        uint16_t t = reader.getTime();
        uint8_t segment = reader.getStrip();

        output.setEnergy(out, energy);
        output.setTime(out, t);
        output.setSegment(out, segment);
    }

    void createOutput(SetupOutput& output, Setup& setup) {
        const auto dCount = setup.dssdCount();
        const auto sCount = setup.singleCount();
        const auto sigCount = setup.signalCount();
        for (size_t i = 0; i < dCount; i++) {
            auto d = setup.getDSSD(i);
            CalibratedOutput front(d, d->frontStripCount());
            CalibratedOutput back(d, d->backStripCount());
            output.addDssdOutput(CalibratedDoubleOutput(d, front, back));
        }


        for (size_t i = 0; i < sCount; i++) {
            auto d = setup.getSingleSided(i);
            CalibratedSingleOutput singleOutput = CalibratedSingleOutput(d, d -> segmentCount());
            singleOutput.setMultiplicity(0);
            output.addSingleOutput(singleOutput);
        }

        for (int i = 0; i < sigCount; i++) {
            output.addScalerOutput(CalibratedSignal{make_unique<UInt_t>(), setup.getScaler().getChannelName(i)});
        }
    }
}

ProtoReader::ProtoReader(std::string path, std::shared_ptr<Setup> setup) :
        dCount(setup -> dssdCount()) , sCount(setup -> singleCount()), sigCount(setup ->signalCount()), path(path)
{
    createOutput(output, *setup);

    // TODO: One should probably read the headers here
}

void ProtoReader::run() {
    kj::FdInputStream fdStream(kj::AutoCloseFd(open(path.c_str(), O_RDONLY)));
    LZ4InputStream bufferedStream(fdStream);

    if (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::InputStreamMessageReader message(bufferedStream);
        auto header = message.getRoot<Header>();
    }

    provider.notifySetup(output);
    capnp::ReaderOptions op;
    kj::Array<capnp::word> ptr = kj::heapArray<capnp::word>(1024);

    while (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::InputStreamMessageReader message(bufferedStream, op, ptr);

        auto evt = message.getRoot<PackedEvent>();
        auto mul = evt.getMul();
        auto data = evt.getData();
        auto sig = evt.getSignal();
        size_t count = 0;


        // Read in DSSD data.
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
                count+=2;
            }
        }

        // Read in single data
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

        // Read in scalers.
        for (size_t i = 0; i < sigCount; i++) {
            output.getScalerOutput(i).setValue(sig[i]);
        }

        provider.notifyAnalyze();
    }

    provider.notifyTerminate();
}

void ProtoReader::saveToRootFile(std::string filename, string options) {
    AbstractCalibratedProvider::provider.notifySaveRoot(filename, options);
}
