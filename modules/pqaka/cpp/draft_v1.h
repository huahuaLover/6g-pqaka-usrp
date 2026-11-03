#pragma once
#include "primitives.h"

namespace pqaka {
struct DraftContextV1 {
  uint16_t suite=1;
  uint16_t key_id=0;
  uint8_t identity_path=0;
  Bytes rand, serving_network, ciphertext;
};
struct DraftKeysV1 { Bytes auth_key, ausf_key, hnmac, kseaf; };
Bytes encode_fields(const std::vector<Bytes>& fields);
std::vector<Bytes> decode_fields(const Bytes& data, size_t count);
Bytes encode_draft_context_v1(const DraftContextV1& context);
DraftKeysV1 derive_draft_v1(const Bytes& k,const Bytes& ks,const DraftContextV1& context);
bool verify_draft_hnmac_v1(const Bytes& k,const Bytes& ks,const DraftContextV1& context,const Bytes& tag);
}
