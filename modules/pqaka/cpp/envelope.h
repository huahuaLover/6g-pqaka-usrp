#pragma once
#include "primitives.h"
#include <map>
namespace pqaka {
struct DraftEnvelope {uint8_t kind,path;uint16_t suite,key_id;std::map<uint8_t,Bytes> fields;};
Bytes encode_draft_envelope(const DraftEnvelope& m);
DraftEnvelope decode_draft_envelope(const Bytes& data);
}
