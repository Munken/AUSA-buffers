#ifndef BUF_PROTO_WRITER_H
#define BUF_PROTO_WRITER_H

#include <match/analyzer/AbstractCalibratedAnalyzer.h>
#include "LZ4OutputStream.h"
#include <capnp/common.h>


namespace AUSA {
    namespace protobuf {
        class
        ProtoWriter : public Match::AbstractCalibratedAnalyzer {
        public:
            ProtoWriter(std::string path,
                    LZ4CompressionLevel compressionLevel = LZ4CompressionLevel::DEFAULT , size_t chunkSize = 20 << 20);
            ~ProtoWriter();

            virtual void setup(Match::CalibratedSetupOutput const &output) override;

            virtual void terminate() override;

            virtual void analyze() override;

        private:
            int fd;

            kj::FdOutputStream* fdStream;
            LZ4OutputStream* bufferedStream;

            kj::Array<capnp::word> buffer;
        };
    }
}

#endif