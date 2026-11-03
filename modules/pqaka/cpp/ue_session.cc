#include "ue_session.h"
#include "identity.h"
#include <oqs/oqs.h>
#include <mbedtls/sha256.h>
#include <mbedtls/platform_util.h>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace pqaka {
static std::string env(const char* name){const char* v=std::getenv(name);return v?v:"";}
std::unique_ptr<UESession> UESession::from_environment(){auto enabled=env("PQAKA_UE_ENABLED");if(enabled.empty()||enabled=="0")return nullptr;if(enabled!="1")throw std::invalid_argument("invalid PQAKA mode");auto path=env("PQAKA_UE_PUBLIC_KEY_FILE"),pin=env("PQAKA_UE_PUBLIC_KEY_SHA256"),id=env("PQAKA_UE_KEY_ID");if(path.empty()||path[0]!='/'||pin.size()!=64||id.empty()||id.size()>3)throw std::invalid_argument("PQAKA UE configuration");for(auto c:id)if(c<'0'||c>'9')throw std::invalid_argument("PQAKA key ID");auto keyid=std::stoul(id);if(keyid>255)throw std::invalid_argument("PQAKA key ID");std::ifstream f(path,std::ios::binary);Bytes pk(801);f.read(reinterpret_cast<char*>(pk.data()),pk.size());if(f.gcount()!=800)throw std::invalid_argument("PQAKA public key size");pk.resize(800);uint8_t digest[32];if(mbedtls_sha256_ret(pk.data(),pk.size(),digest,0)!=0)throw std::runtime_error("SHA256 failed");std::ostringstream h;for(auto x:digest)h<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(x);if(h.str()!=pin)throw std::invalid_argument("PQAKA public key pin");return std::unique_ptr<UESession>(new UESession(pk,uint16_t(keyid)));}
UESession::~UESession(){mbedtls_platform_zeroize(ks_.data(),ks_.size());}
Bytes UESession::begin(const std::string& msin,bool guti){std::unique_ptr<OQS_KEM,decltype(&OQS_KEM_free)> kem(OQS_KEM_new(OQS_KEM_alg_ml_kem_512),OQS_KEM_free);if(!kem||public_key_.size()!=kem->length_public_key)throw std::runtime_error("ML-KEM unavailable");mbedtls_platform_zeroize(ks_.data(),ks_.size());ks_.resize(32);c_.resize(768);if(OQS_KEM_encaps(kem.get(),c_.data(),ks_.data(),public_key_.data())!=OQS_SUCCESS)throw std::runtime_error("ML-KEM encapsulation");initial_=DraftEnvelope{1,uint8_t(guti?2:1),1,key_id_,{{1,c_}}};if(!guti){auto p=protect_draft_identity(ks_,c_,msin);initial_.fields[2]=p.ct;initial_.fields[3]=p.tag;}return encode_draft_envelope(initial_);}
DraftEnvelope UESession::check_challenge(const Bytes& wire,const Bytes& rand)const{if(ks_.size()!=32)throw std::invalid_argument("no PQAKA session");auto e=decode_draft_envelope(wire);if(e.kind!=2||e.path!=initial_.path||e.suite!=initial_.suite||e.key_id!=key_id_||e.fields.at(4)!=rand)throw std::invalid_argument("PQAKA challenge context mismatch");return e;}
}
