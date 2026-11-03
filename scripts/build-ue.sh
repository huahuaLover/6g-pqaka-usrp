#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
rev=f4b96220e4bd208895172acc4fedb5a191d9f5b1
if [[ ! -d "$root/sources/liboqs/.git" ]];then
 [[ ! -e "$root/sources/liboqs" ]] || { echo 'liboqs path exists without Git metadata; inspect it first.' >&2;exit 1; }
 git clone --branch 0.12.0 --depth 1 https://github.com/open-quantum-safe/liboqs.git "$root/sources/liboqs"
fi
[[ "$(git -C "$root/sources/liboqs" rev-parse HEAD)" == "$rev" ]] || { echo 'liboqs version mismatch.' >&2;exit 1; }
[[ -z "$(git -C "$root/sources/liboqs" status --porcelain)" ]] || { echo 'liboqs has local changes.' >&2;exit 1; }
cmake -S "$root/sources/liboqs" -B "$root/artifacts/liboqs-build" \
 -DOQS_MINIMAL_BUILD=KEM_ml_kem_512 -DOQS_BUILD_ONLY_LIB=ON \
 -DOQS_USE_OPENSSL=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build "$root/artifacts/liboqs-build" -j "${PQAKA_BUILD_JOBS:-2}"
cmake -S "$root/sources/srsRAN_4G" -B "$root/artifacts/srsue-uhd-build" \
 -DPQAKA_ROOT="$root" -DCMAKE_BUILD_TYPE=Release -DENABLE_UHD=ON \
 -DENABLE_ZEROMQ=OFF -DENABLE_BLADERF=OFF -DENABLE_SOAPYSDR=OFF \
 -DENABLE_SRSENB=OFF -DENABLE_SRSEPC=OFF
# ENABLE_UHD alone is insufficient if discovery failed: check CMake's result too.
grep -q '^UHD_FOUND:INTERNAL=1' "$root/artifacts/srsue-uhd-build/CMakeCache.txt" || \
 grep -q '^UHD_LIBRARIES:.*libuhd' "$root/artifacts/srsue-uhd-build/CMakeCache.txt" || {
 echo 'UHD was not found; install its development package and inspect CMake output.' >&2;exit 1;
}
cmake --build "$root/artifacts/srsue-uhd-build" --target srsue -j "${PQAKA_BUILD_JOBS:-2}"
