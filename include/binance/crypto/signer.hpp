#ifndef BINANCE_SIGNER_HPP
#define BINANCE_SIGNER_HPP

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <binance/conv.hpp>
#include <boost/utility/string_view.hpp>
#include <iostream>
#include <string>
#include <string_view>

namespace binance
{
namespace crypto
{
class signer
{
  ::HMAC_CTX* hmac_;
  binance::conv::hex hex_;

public:
  signer() = delete;
  signer(const std::string& key)
      : hmac_(::HMAC_CTX_new())
  {
    ::HMAC_Init_ex(hmac_, &key[0], key.size(), ::EVP_sha256(), nullptr);
  }
  ~signer()
  {
    ::HMAC_CTX_free(hmac_);
  }
  void update(const boost::string_view& s)
  {
    ::HMAC_Update(hmac_, (unsigned char*) &s[0], s.size());
  }
  const std::string& final()
  {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int size;

    ::HMAC_Final(hmac_, &hash[0], &size);

    return hex_.clear().encode(&hash[0], size).final();
  }
};
}  // namespace crypto
}  // namespace binance

#endif
