#include "draft_v1.h"
#include <mbedtls/md.h>
#include <stdexcept>

namespace pqaka {
namespace {
Bytes text(const char* s) { const std::string value(s); return Bytes(value.begin(),value.end()); }
Bytes u16(uint16_t n) { return {static_cast<uint8_t>(n>>8),static_cast<uint8_t>(n)}; }
bool valid_network(const Bytes& network) {
  if (network.empty() || network.size()>255) return false;
  for (auto c:network) if (c<0x21 || c>0x7e) return false;
  return true;
}
}
Bytes encode_fields(const std::vector<Bytes>& fields) {
  Bytes out;
  for (const auto& field:fields) {
    if (field.size()>65535) throw std::invalid_argument("field exceeds 16-bit length");
    out.insert(out.end(),field.begin(),field.end());
    const auto length=u16(static_cast<uint16_t>(field.size()));
    out.insert(out.end(),length.begin(),length.end());
  }
  return out;
}
std::vector<Bytes> decode_fields(const Bytes& data,size_t count) {
  if (count>32) throw std::invalid_argument("invalid field count");
  std::vector<Bytes> fields(count);
  size_t end=data.size();
  for (size_t i=count;i>0;--i) {
    if (end<2) throw std::invalid_argument("truncated field length");
    const size_t n=(static_cast<size_t>(data[end-2])<<8)|data[end-1];
    end-=2;
    if (n>end) throw std::invalid_argument("truncated field");
    fields[i-1]=Bytes(data.begin()+end-n,data.begin()+end);
    end-=n;
  }
  if (end) throw std::invalid_argument("unexpected leading data or field count");
  return fields;
}
Bytes encode_draft_context_v1(const DraftContextV1& c) {
  if (c.suite!=1 || (c.identity_path!=1 && c.identity_path!=2) ||
      c.rand.size()!=16 || c.ciphertext.size()!=768 || !valid_network(c.serving_network))
    throw std::invalid_argument("invalid draft context input");
  Bytes digest(32);
  const auto* md=mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  if (!md || mbedtls_md(md,c.ciphertext.data(),c.ciphertext.size(),digest.data()))
    throw std::runtime_error("SHA-256 failed");
  return encode_fields({text("6G-PQAKA-v1"),u16(c.suite),u16(c.key_id),{c.identity_path},c.rand,c.serving_network,digest});
}
DraftKeysV1 derive_draft_v1(const Bytes& k,const Bytes& ks,const DraftContextV1& c) {
  if (k.size()!=16 || ks.size()!=32) throw std::invalid_argument("invalid K or KS length");
  const auto context=encode_draft_context_v1(c);
  DraftKeysV1 keys;
  keys.auth_key=hkdf_sha256(k,ks,encode_fields({text("HN-auth-key"),context}),32);
  keys.ausf_key=hkdf_sha256(k,ks,encode_fields({text("K-AUSF-PQ"),context}),32);
  keys.hnmac=hmac_sha256(keys.auth_key,encode_fields({text("HN-auth"),context}));
  keys.kseaf=kdf_3gpp(keys.ausf_key,0x6c,{c.serving_network});
  return keys;
}
bool verify_draft_hnmac_v1(const Bytes& k,const Bytes& ks,const DraftContextV1& c,const Bytes& tag) {
  if (tag.size()!=32) return false;
  try {
    const auto expected=derive_draft_v1(k,ks,c).hnmac;
    // Fixed-length comparison without a data-dependent early exit.
    volatile uint8_t difference=0;
    for (size_t i=0;i<32;++i) difference=static_cast<uint8_t>(difference|(expected[i]^tag[i]));
    return difference==0;
  } catch (const std::invalid_argument&) { return false; }
}
}
