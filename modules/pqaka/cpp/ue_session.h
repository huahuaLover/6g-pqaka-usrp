#pragma once
#include "envelope.h"
#include <memory>
#include <string>
namespace pqaka {
class UESession {
 Bytes public_key_,ks_,c_;
 DraftEnvelope initial_;
 uint16_t key_id_;
public:
 static std::unique_ptr<UESession> from_environment();
 UESession(Bytes public_key,uint16_t key_id):public_key_(std::move(public_key)),key_id_(key_id){}
 ~UESession();
 Bytes begin(const std::string& msin,bool guti);
 DraftEnvelope check_challenge(const Bytes& wire,const Bytes& rand)const;
 const Bytes& ks()const{return ks_;} const Bytes& ciphertext()const{return c_;}
 uint16_t key_id()const{return key_id_;}uint8_t path()const{return initial_.path;}
};
}
