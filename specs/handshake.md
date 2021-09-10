## Handshake process
Encryption in openSIMP is enforced and as such the handshake process
is outlined here step by step.

- Server starts
	- Generates new RSA keypair on first start
	- Generates an AES key on first start
- Client joins
	- Client needs to generate their keypair on startup
	- Reserve space for AES key
- Server sends public key to client
- Client encrypts the auth packet with the server's public key
	- The auth packet is structured like "05phnixirpassword"
	- A 2 digit number at the start represents the username length and is padded with zeroes if its only 1 digit
	- The username follows after the padded number and so does the password
	- All of this will be encrypted with the server's public key and sent safely
- If the server accepts the authentication continue with the steps otherwise kick (planned retrying functionality instead of kicking)
- Client sends their public key to server (without encrypting it using the server public key)
- Server encrypts the AES key with the public key of the client (after auth was successful) and sends it back to the client
- Client decrypts the ciphertext which is the AES key with their private key and save the AES key in a variable or something

The handshake is now done and all communication is secured.
