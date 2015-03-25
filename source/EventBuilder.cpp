#include "ausa/buf/PackedEvent.capn.h"
#include "ausa/buf/EventBuilder.h"

using namespace std;

using namespace AUSA::protobuf;
using namespace AUSA::Match;

namespace {
    pair<UInt_t, UInt_t> writeMul(const SetupOutput& output, ::capnp::List< ::uint8_t>::Builder builder) {
        UInt_t dSum = 0, sSum = 0;
        int dsdCount = output.dssdCount();
        for (int i = 0; i < dsdCount; i++) {
            UInt_t m = output.getDssdOutput(i).front().multiplicity();
            dSum += m;
            builder.set((unsigned int) i, (uint8_t) m);
        }
        
        for (int i = 0; i < output.singleCount(); i++) {
            UInt_t m = output.getSingleOutput(i).multiplicity();
            builder.set((unsigned int) (dsdCount+i), (uint8_t) m);
            sSum += m;
        }

        return make_pair(dSum, sSum);
    }

    void writeOutput(const int i, const CalibratedOutput& out , Data::Builder& builder) {
        uint16_t time = (uint16_t) out.time(i);
        double energy = out.energy(i);
        uint8_t segment = (uint8_t) out.segment(i);

//        cout << "Write: (T, E, S) " << time << " " << energy << " " << to_string(segment) << endl;

        builder.setTime(time);
        builder.setEnergy(energy);
        builder.setStrip(segment);
    }
}

void AUSA::protobuf::buildEvent(capnp::MessageBuilder& builder, const SetupOutput &output) {
    auto event = builder.initRoot<PackedEvent>();

    auto mulList = event.initMul(output.dssdCount() + output.singleCount());
    auto mul = writeMul(output, mulList);

    auto doubleMul = mul.first; auto singleMul = mul.second;

    auto data = event.initData(2*doubleMul+singleMul);

    size_t count = 0;
    for (size_t i = 0; i < output.dssdCount(); i++) {
        auto& out = output.getDssdOutput(i);
        for (size_t j = 0; j < mulList[i]; j++) {
            Data::Builder bf = data[count];
            writeOutput(j, out.front(), bf);
            Data::Builder bb = data[count+1];
            writeOutput(j, out.back(), bb);

            count+=2;
        }
    }

    for (size_t i = 0; i < output.singleCount(); i++) {
        auto& out = output.getSingleOutput(i);
        for (size_t j = 0; j < mulList[i+output.dssdCount()]; j++) {
            Data::Builder b = data[count];
            writeOutput(j, out, b);

            count+=1;
        }
    }


//    if (doubleMul > 0 || singleMul > 0) {
//        cout << doubleMul << "\t" << singleMul << endl;
//        auto reader = data.asReader();
//        cout << "Reader!" << endl;
//        for (auto r : reader) {
//            cout << "Read: (T, E, S) " << r.getTime() << " " << r.getEnergy() << " " << to_string(r.getStrip()) << endl;
//        }
//        cout << endl;
//    }

    auto signals = event.initSignal(output.signalCount());
    for (size_t i = 0; i < output.signalCount(); i++) {
        signals.set(i, output.getScalerOutput(i).getValue());
    }
}


