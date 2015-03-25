#ifndef BUF_HEADER_BUILDER_H
#define BUF_HEADER_BUILDER_H

#include "Header.capn.h"
#include <ausa/output/GenericSetupOutput.h>
#include <ausa/AUSA.h>

#include <capnp/message.h>
#include <memory>

namespace AUSA {
    namespace buf {

        /**
        * Given a reference to a GenericSetupOutput build a AUSA buffer header.
        */
        template <class Single, class Double, class Signal>
        void buildHeader(capnp::MessageBuilder& builder, const ::AUSA::Output::GenericSetupOutput<Single, Double, Signal>& output) {
            auto header = builder.initRoot<Header>();

            auto doubles = header.initDoubles(output.dssdCount());
            for (size_t i = 0; i < doubles.size(); i++) {
                auto& d = output.getDssdOutput(i).detector();
                doubles[i].setName(d.getName());
                doubles[i].setBackStrips(d.backStripCount());
                doubles[i].setFrontStrips(d.frontStripCount());
            }

            auto singles = header.initSingles(output.singleCount());
            for (size_t i = 0; i < singles.size(); i++) {
                auto& d = output.getSingleOutput(i).detector();
                singles[i].setName(d.getName());
                singles[i].setStrips(d.segmentCount());
            }

            auto signals = header.initSignals(output.signalCount());
            for (size_t i = 0; i < signals.size(); i++) {
                auto& d = output.getScalerOutput(i);
                signals[i].setName(d.getName());
            }
        }
    }
}

#endif