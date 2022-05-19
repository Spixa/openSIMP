#include <botan/rng.h>
#include <botan/auto_rng.h>
#include <botan/cipher_mode.h>
#include <botan/hex.h>
#include <iostream>
#include <botan/pkcs8.h>
#include <botan/pk_keys.h>
#include <botan/pubkey.h>
#include <botan/rsa.h>
#include <memory.h>
#include <cstdlib>
#include <memory>
#include <unordered_map>


#define ERROR_MSG "an internal error occured whilst executing the program"

const int EXPONENT_MAX = 65537;

using namespace Botan;
class Cryptography {
public:
    Cryptography();
    secure_vector<uint8_t> encrypt(const std::string & str, const char keyhex[]);
    std::unordered_map<std::string, secure_vector<uint8_t>> decrypt(const secure_vector<uint8_t>& ct, const char keyhex[]);
    BigInt getPublicKey() {
        return kp.get()->get_n();
    }
    void pushNewClientKey(const std::string& key);
    void pushNewClientKey(const BigInt& bi);
    const std::vector<uint8_t>   RSA_encrypt(size_t iterator, const std::string&);
    secure_vector<uint8_t> RSA_decrypt(secure_vector<uint8_t>);

public: 
    // Public key container
    std::vector<RSA_PublicKey> keys;
private:
    std::unique_ptr<Cipher_Mode> enc;
    std::unique_ptr<Cipher_Mode> dec;
    AutoSeeded_RNG rng;
    std::unique_ptr<Botan::RSA_PrivateKey> kp;

};
