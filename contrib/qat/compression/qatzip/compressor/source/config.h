#pragma once

#include "envoy/compression/compressor/config.h"
#include "envoy/compression/compressor/factory.h"
#include "envoy/thread_local/thread_local.h"

#include "source/common/http/headers.h"

#include "contrib/envoy/extensions/compression/qatzip/compressor/v3alpha/qatzip.pb.h"
#include "contrib/envoy/extensions/compression/qatzip/compressor/v3alpha/qatzip.pb.validate.h"
#include "contrib/qat/compression/qatzip/compressor/source/qatzip_compressor_impl.h"

namespace Envoy {
namespace Extensions {
namespace Compression {
namespace Qatzip {
namespace Compressor {

namespace {

const std::string& qatzipStatsPrefix() { CONSTRUCT_ON_FIRST_USE(std::string, "qatzip."); }
const std::string& qatzipExtensionName() {
  CONSTRUCT_ON_FIRST_USE(std::string, "envoy.compression.qatzip.compressor");
}

} // namespace

class QatzipCompressorFactory : public Envoy::Compression::Compressor::CompressorFactory {
public:
  QatzipCompressorFactory(
      const envoy::extensions::compression::qatzip::compressor::v3alpha::Qatzip& qatzip,
      Server::Configuration::FactoryContext& context);

  // Envoy::Compression::Compressor::CompressorFactory
  Envoy::Compression::Compressor::CompressorPtr createCompressor() override;
  const std::string& statsPrefix() const override { return qatzipStatsPrefix(); }
  const std::string& contentEncoding() const override {
    return Http::CustomHeaders::get().ContentEncodingValues.Gzip;
  }

private:
  struct QatzipThreadLocal : public ThreadLocal::ThreadLocalObject {
    QatzipThreadLocal(QzSessionParams_T params);
    ~QatzipThreadLocal() override;
    QzSession_T* getSession();

    QzSessionParams_T params_;
    QzSession_T session_;
    bool initialized_;
  };

  const uint32_t chunk_size_;
  ThreadLocal::SlotPtr tls_slot_;
};

class QatzipCompressorLibraryFactory
    : public Envoy::Compression::Compressor::NamedCompressorLibraryConfigFactory {
public:
  QatzipCompressorLibraryFactory() : name_{qatzipExtensionName()} {}

  Envoy::Compression::Compressor::CompressorFactoryPtr
  createCompressorFactoryFromProto(const Protobuf::Message& proto_config,
                                   Server::Configuration::FactoryContext& context) override {
    return createCompressorFactoryFromProtoTyped(
        MessageUtil::downcastAndValidate<
            const envoy::extensions::compression::qatzip::compressor::v3alpha::Qatzip&>(
            proto_config, context.messageValidationVisitor()),
        context);
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return std::make_unique<envoy::extensions::compression::qatzip::compressor::v3alpha::Qatzip>();
  }

  std::string name() const override { return name_; }

private:
  Envoy::Compression::Compressor::CompressorFactoryPtr createCompressorFactoryFromProtoTyped(
      const envoy::extensions::compression::qatzip::compressor::v3alpha::Qatzip& config,
      Server::Configuration::FactoryContext& context);

  const std::string name_;
};

DECLARE_FACTORY(QatzipCompressorLibraryFactory);

} // namespace Compressor
} // namespace Qatzip
} // namespace Compression
} // namespace Extensions
} // namespace Envoy
