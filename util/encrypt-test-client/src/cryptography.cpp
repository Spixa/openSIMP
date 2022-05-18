#include "cryptography.h"

Cryptography::Cryptography() : rng(), kp(new Botan::RSA_PrivateKey(rng, 4096)) {
      enc = Cipher_Mode::create("AES-256/GCM", ENCRYPTION);
      dec = Cipher_Mode::create("AES-256/GCM", DECRYPTION); 
} 

secure_vector<uint8_t> Cryptography::encrypt(const std::string & str, const char keyhex[]) {
             // defining variables
             secure_vector<uint8_t> input;
             secure_vector<uint8_t> key;
            try {
              key = hex_decode_locked(keyhex);
            } catch (std::exception e) {
                  std::cout << ERROR_MSG << '\n' << "\twhat(): "
                  << e.what() << '\n';
            }

            input = secure_vector<uint8_t>(str.data(), str.data() + str.length());
            secure_vector<uint8_t> iv = this->rng.random_vec(enc->default_nonce_length()); 

         // start to compile the string into a ciphertext
         enc->set_key(key);
         enc->start(iv);
         enc->finish(input);
         std::string res_str{hex_encode(iv) + hex_encode(input)}; 
         return hex_decode_locked(res_str);
}
std::unordered_map<std::string, secure_vector<uint8_t>> Cryptography::decrypt(const secure_vector<uint8_t>& ct, const char keyhex[]) {
          secure_vector<uint8_t> output;
          secure_vector<uint8_t> iv;
          secure_vector<uint8_t> key;
            try {
             key = hex_decode_locked(keyhex);
            } catch (std::exception e) {
                  std::cout << ERROR_MSG << '\n' << "\twhat(): "
                  << e.what() << '\n';
            }
            
            for (int i = 0; i < 12; i++) {
                  iv.push_back(ct[i]);
            }

            for (int i = 12; i < ct.size(); i++) {
                  output.push_back(ct[i]);
            }


           // start to decompile 
           dec->set_key(key);
           dec->start(iv);

           dec->finish(output, 0); 

          std::unordered_map<std::string, secure_vector<uint8_t>> map;
          map["iv"] = iv;
          map["msg"] = output; 

         return map;
}


void Cryptography::pushNewClientKey(const std::string& key) {
      keys.push_back(RSA_PublicKey( BigInt(hex_decode(key)), EXPONENT_MAX));

}

void Cryptography::pushNewClientKey(const BigInt& bi) {
      keys.push_back(RSA_PublicKey( bi, EXPONENT_MAX));
}

const std::vector<uint8_t> Cryptography::RSA_encrypt(size_t iterator, const std::string& plaintext) {
      // Encrypter device

      PK_Encryptor_EME enc_dev(keys[iterator], rng, "PKCS1v15");
      

      secure_vector<uint8_t> pt(plaintext.data(), plaintext.data() + plaintext.length());
      std::vector<uint8_t> ct = enc_dev.encrypt(pt, rng);
      

      return ct;
}

secure_vector<uint8_t> Cryptography::RSA_decrypt(secure_vector<uint8_t> given) {
      // Decrypt deviice
      PK_Decryptor_EME dec_dev(*kp, rng, "PKCS1v15");

      return dec_dev.decrypt(given);
}
