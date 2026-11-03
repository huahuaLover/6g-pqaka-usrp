#include "response.h"
#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>
#include <stdexcept>
namespace pqaka {
MilenageResponse milenage_response(const Bytes& k,const Bytes& opc,const Bytes& rand) {
 if(k.size()!=16 || opc.size()!=16 || rand.size()!=16) throw std::invalid_argument("K/OPc/RAND length");
 mbedtls_aes_context aes; mbedtls_aes_init(&aes);
 if(mbedtls_aes_setkey_enc(&aes,k.data(),128)!=0) {mbedtls_aes_free(&aes);throw std::runtime_error("AES key");}
 Bytes temp(16); for(size_t i=0;i<16;i++)temp[i]=rand[i]^opc[i];
 if(mbedtls_aes_crypt_ecb(&aes,MBEDTLS_AES_ENCRYPT,temp.data(),temp.data())!=0){mbedtls_aes_free(&aes);throw std::runtime_error("AES temp");}
 auto output=[&](size_t rotation,uint8_t constant){
  Bytes in(16);for(size_t i=0;i<16;i++)in[i]=temp[(i+rotation)%16]^opc[(i+rotation)%16];in[15]^=constant;
  int rc=mbedtls_aes_crypt_ecb(&aes,MBEDTLS_AES_ENCRYPT,in.data(),in.data());
  if(rc!=0){mbedtls_aes_free(&aes);throw std::runtime_error("AES output");}
  for(size_t i=0;i<16;i++)in[i]^=opc[i];
  return in;
 };
 Bytes out=output(0,1);MilenageResponse result{Bytes(out.begin()+8,out.end()),output(4,2),output(8,4)};
 mbedtls_aes_free(&aes);return result;
}
Bytes derive_res_star(const Bytes& ck,const Bytes& ik,const Bytes& rand,const Bytes& res,const Bytes& sn){
 if(ck.size()!=16||ik.size()!=16||rand.size()!=16||res.size()!=8||sn.empty()||sn.size()>255)throw std::invalid_argument("response length");
 for(auto c:sn)if(c<0x21||c>0x7e)throw std::invalid_argument("SN bytes");
 Bytes key=ck;key.insert(key.end(),ik.begin(),ik.end());auto full=kdf_3gpp(key,0x6b,{sn,rand,res});return Bytes(full.begin()+16,full.end());
}
Bytes hash_res_star(const Bytes& rand,const Bytes& response){
 if(rand.size()!=16||response.size()!=16)throw std::invalid_argument("RAND/RES* length");
 Bytes in=rand;in.insert(in.end(),response.begin(),response.end());Bytes sum(32);
 if(mbedtls_sha256_ret(in.data(),in.size(),sum.data(),0)!=0)throw std::runtime_error("SHA256");
 return Bytes(sum.begin()+16,sum.end());
}
}
