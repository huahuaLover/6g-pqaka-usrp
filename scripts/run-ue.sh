#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
if (( EUID != 0 ));then exec sudo "$0" "$@";fi
cd "$root"
[[ -x artifacts/srsue-uhd-build/srsue/src/srsue ]] || { echo 'Build UE first.' >&2;exit 1; }
[[ -r secrets/ue/runtime.env && -r config/ue-b210.local.conf ]] || { echo 'Private UE configuration is missing.' >&2;exit 1; }
set -a
source secrets/ue/runtime.env
set +a
[[ "${PQAKA_UE_ENABLED:-}" == 1 ]] || { echo 'PQAKA_UE_ENABLED must be 1.' >&2;exit 1; }
if ! ip netns list | awk '{print $1}' | grep -qx pqaka-ue;then ip netns add pqaka-ue;fi
ip netns exec pqaka-ue ip link set lo up
mkdir -p logs
exec artifacts/srsue-uhd-build/srsue/src/srsue config/ue-b210.local.conf
