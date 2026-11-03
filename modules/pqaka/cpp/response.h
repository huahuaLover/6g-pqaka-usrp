#pragma once
#include "primitives.h"
namespace pqaka {
struct MilenageResponse { Bytes res, ck, ik; };
MilenageResponse milenage_response(const Bytes& k,const Bytes& opc,const Bytes& rand);
Bytes derive_res_star(const Bytes& ck,const Bytes& ik,const Bytes& rand,const Bytes& res,const Bytes& sn);
Bytes hash_res_star(const Bytes& rand,const Bytes& response);
}
