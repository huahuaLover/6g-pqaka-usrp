#include "envelope.h"
#include <stdexcept>
namespace pqaka {
static void invalid(){throw std::invalid_argument("invalid draft PQAKA envelope");}
static void validate(const DraftEnvelope& m){if(m.suite!=1||(m.path!=1&&m.path!=2))invalid();std::map<uint8_t,int> expected;
 switch(m.kind){case 1:expected[1]=768;if(m.path==1){expected[2]=-1;expected[3]=8;}break;case 2:expected[4]=16;expected[5]=32;break;case 3:expected[6]=16;break;default:invalid();}
 if(expected.size()!=m.fields.size()){invalid();}
 for(auto p:expected){auto it=m.fields.find(p.first);if(it==m.fields.end())invalid();size_t n=it->second.size();if(p.second==-1){if(n<1||n>5)invalid();}else if(n!=size_t(p.second))invalid();}}
Bytes encode_draft_envelope(const DraftEnvelope& m){validate(m);Bytes out{'P','Q','A','K',1,m.kind,m.path,uint8_t(m.suite>>8),uint8_t(m.suite),uint8_t(m.key_id>>8),uint8_t(m.key_id),0,0};for(auto p:m.fields){out.push_back(p.first);out.push_back(uint8_t(p.second.size()>>8));out.push_back(uint8_t(p.second.size()));out.insert(out.end(),p.second.begin(),p.second.end());}if(out.size()>1024)invalid();size_t n=out.size()-13;out[11]=uint8_t(n>>8);out[12]=uint8_t(n);return out;}
DraftEnvelope decode_draft_envelope(const Bytes& b){if(b.size()<13||b.size()>1024||b[0]!='P'||b[1]!='Q'||b[2]!='A'||b[3]!='K'||b[4]!=1||size_t((b[11]<<8)|b[12])!=b.size()-13)invalid();DraftEnvelope m{b[5],b[6],uint16_t((b[7]<<8)|b[8]),uint16_t((b[9]<<8)|b[10]),{}};size_t off=13;uint8_t last=0;while(off<b.size()){if(b.size()-off<3)invalid();uint8_t id=b[off];size_t n=(b[off+1]<<8)|b[off+2];off+=3;if(id<=last||id>6||n>b.size()-off)invalid();last=id;m.fields[id]=Bytes(b.begin()+off,b.begin()+off+n);off+=n;}validate(m);return m;}
}
