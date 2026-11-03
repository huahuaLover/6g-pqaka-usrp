#include "primitives.h"
#include <mbedtls/hkdf.h>
#include <mbedtls/md.h>
#include <stdexcept>

namespace pqaka {
namespace {
const mbedtls_md_info_t* sha256() {
  const auto* md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  if (!md) throw std::runtime_error("SHA-256 unavailable");
  return md;
}
}
Bytes hmac_sha256(const Bytes& key, const Bytes& message) {
  Bytes out(32);
  if (mbedtls_md_hmac(sha256(), key.data(), key.size(), message.data(), message.size(), out.data()))
    throw std::runtime_error("HMAC failed");
  return out;
}
Bytes hkdf_sha256(const Bytes& ikm, const Bytes& salt, const Bytes& info, size_t length) {
  if (length > 255*32) throw std::invalid_argument("invalid HKDF output length");
  if (!length) return {};
  Bytes out(length);
  if (mbedtls_hkdf(sha256(), salt.empty() ? nullptr : salt.data(), salt.size(),
                   ikm.data(), ikm.size(), info.data(), info.size(), out.data(), out.size()))
    throw std::runtime_error("HKDF failed");
  return out;
}
Bytes kdf_3gpp(const Bytes& key, uint8_t fc, const std::vector<Bytes>& parameters) {
  Bytes input{fc};
  for (const auto& p : parameters) {
    if (p.size()>65535) throw std::invalid_argument("3GPP KDF parameter exceeds 16-bit length");
    input.insert(input.end(),p.begin(),p.end());
    input.push_back(static_cast<uint8_t>(p.size()>>8));
    input.push_back(static_cast<uint8_t>(p.size()));
  }
  return hmac_sha256(key,input);
}
}
