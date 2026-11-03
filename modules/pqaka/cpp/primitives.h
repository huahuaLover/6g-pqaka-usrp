#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

// Unintegrated generic primitives; not a frozen PQAKA key schedule.
namespace pqaka {
using Bytes = std::vector<uint8_t>;
Bytes hmac_sha256(const Bytes& key, const Bytes& message);
Bytes hkdf_sha256(const Bytes& ikm, const Bytes& salt, const Bytes& info, size_t length);
Bytes kdf_3gpp(const Bytes& key, uint8_t fc, const std::vector<Bytes>& parameters);
}
