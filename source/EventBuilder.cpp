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
}

void AUSA::protobuf::buildEvent(capnp::MessageBuilder& builder, const SetupOutput &output) {
    auto event = builder.initRoot<PackedEvent>();

    auto mulList = event.initMul(output.dssdCount() + output.singleCount());
    auto mul = writeMul(output, mulList);

    auto E = event.initEnergy(mul);
    auto t = event.initTime(mul);
    auto s = event.initStrip(mul);

    size_t count = 0;
    for (size_t i = 0; i < output.dssdCount(); i++) {
        auto& out = output.getDssdOutput(i);
        for (size_t j = 0; j < mulList[i]; j++) {
            E.set(count, out.front().energy(j));
            E.set(count+1, out.back().energy(j));

            t.set(count, out.front().time(j));
            t.set(count+1, out.back().time(j));

            s.set(count, out.front().segment(j));
            s.set(count+1, out.back().segment(j));

            count+=2;
        }
    }

    for (size_t i = 0; i < output.dssdCount(); i++) {
        auto& out = output.getSingleOutput(i);
        for (size_t j = 0; j < mulList[i]; j++) {
            E.set(count, out.energy(j));
            t.set(count, out.time(j));
            s.set(count, out.segment(j));

            count+=1;
        }
    }
}


