#include "buf/PackedEvent.capn.h"
#include "buf/EventBuilder.h"



using namespace AUSA::protobuf;
using namespace AUSA::Match;

namespace {
    UInt_t writeMul(const SetupOutput& output, ::capnp::List< ::uint8_t>::Builder builder) {
        UInt_t sum = 0;
        int dsdCount = output.dssdCount();
        for (int i = 0; i < dsdCount; i++) {
            UInt_t m = output.getDssdOutput(i).front().multiplicity();
            sum += m;
            builder.set(i, m);
        }
        
        for (int i = 0; i < output.singleCount(); i++) {
            UInt_t m = output.getSingleOutput(i).multiplicity();
            builder.set(dsdCount+i, m);
            sum += m;
        }
        return sum;
    }

    void writeOutput(const int i, const CalibratedOutput& out , Data::Builder& builder) {
        builder.setTime(out.time(i));
        builder.setEnergy(out.energy(i));
        builder.setStrip(out.segment(i));
    }
}

void AUSA::protobuf::buildEvent(capnp::MessageBuilder& builder, const SetupOutput &output) {
    auto event = builder.initRoot<PackedEvent>();

    auto mulList = event.initMul(output.dssdCount() + output.singleCount());
    auto mul = writeMul(output, mulList);

    auto data = event.initData(mul);

    size_t count = 0;
    for (size_t i = 0; i < output.dssdCount(); i++) {
        auto& out = output.getDssdOutput(i);
        for (size_t j = 0; j < mulList[i]; j++) {
            Data::Builder bf = data[count];
            writeOutput(count, out.front(), bf);
            Data::Builder bb = data[count+1];
            writeOutput(count, out.back(), bb);

            count+=2;
        }
    }

    for (size_t i = 0; i < output.dssdCount(); i++) {
        auto& out = output.getSingleOutput(i);
        for (size_t j = 0; j < mulList[i]; j++) {
            Data::Builder b = data[count];
            writeOutput(count, out, b);

            count+=1;
        }
    }
}


