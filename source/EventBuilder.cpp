#include "buf/Event.capn.h"
#include "buf/EventBuilder.h"

#include <capnp/message.h>
#include <match/output.h>

using namespace AUSA::protobuf;
using namespace AUSA::Match;

namespace {
    void buildSingle(::AUSA::protobuf::SingleOutput::Builder& b, const CalibratedOutput& s) {
        auto mul = s.multiplicity();
        b.setMul(mul);

        auto strips = b.initStrip(mul);
        auto E = b.initEnergy(mul);
        auto tdc = b.initTdc(mul);

        for (int i = 0; i < mul; i++) {
            strips.set(i, s.segment(i));
            E.set(i, s.energy(i));
            tdc.set(i, s.time(i));
        }
    }
}

void ::AUSA::protobuf::buildEvent(capnp::MessageBuilder& builder, const SetupOutput &output) {
    auto event = builder.initRoot<Event>();

    auto doubles = event.initDoubleOutput(output.dssdCount());
    for (size_t i = 0; i < doubles.size(); i++) {
        auto& out = output.getDssdOutput(i);

        DoubleOutput::Builder b = doubles[0];
        SingleOutput::Builder front = b.getFront();
        buildSingle(front, out.front());
        SingleOutput::Builder back = b.getBack();
        buildSingle(back, out.back());
    }

    auto singles = event.initSingleOutput(output.singleCount());
    for (size_t i = 0; i < singles.size(); i++) {
        SingleOutput::Builder builder = singles[0];
        buildSingle(builder, output.getSingleOutput(i));
    }

    auto signals = event.initSignalOutput(output.signalCount());
    for (size_t i = 0; i < signals.size(); i++) {
        SignalOutput::Builder builder = signals[0];
        builder.setValue(output.getScalerOutput(i).getValue());
    }
}
