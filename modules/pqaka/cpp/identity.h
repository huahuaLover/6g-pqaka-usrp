#pragma once
#include "primitives.h"
#include <string>
namespace pqaka {
Bytes encode_msin(const std::string& s);
std::string decode_msin(const Bytes& p);
struct IdentityKeys {Bytes ke,icb,km;};
struct ProtectedIdentity {Bytes ct,tag;};
IdentityKeys derive_draft_identity_keys(const Bytes& ks,const Bytes& c);
Bytes identity_ctr(const Bytes& key,const Bytes& icb,const Bytes& p);
ProtectedIdentity protect_draft_identity(const Bytes& ks,const Bytes& c,const std::string& msin);
std::string recover_draft_identity(const Bytes& ks,const Bytes& c,const Bytes& ct,const Bytes& tag);
}
