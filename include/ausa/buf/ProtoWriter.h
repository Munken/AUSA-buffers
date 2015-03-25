#ifndef BUF_PROTO_WRITER_H
#define BUF_PROTO_WRITER_H

#include "LZ4OutputStream.h"

#include <ausa/match/analyzer/AbstractCalibratedAnalyzer.h>
#include <capnp/common.h>


namespace AUSA {
    namespace protobuf {

        /**
        * Analyzer that takes a stream of events and writes them to a AUSA buffer file.
        *
        * Internally it uses a LZ4OutputStream to write compressed data to a file.
        */
        class ProtoWriter : public Match::AbstractCalibratedAnalyzer {
        public:

            /**
            * Create a ProtoWriter that will write to the specified file.
            *
            * Data is written chunks which size in bytes is specified by chunkSize.
            *
            * The default compression = fast.
            */
            ProtoWriter(std::string path,
                    LZ4CompressionLevel compressionLevel = LZ4CompressionLevel::DEFAULT , size_t chunkSize = 50 << 20 /*50MB*/);
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