/*
Copyright 2011 Benjamin Fus, Florian Muenchbach, Mathias Gottschlag. All
rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PUBLICKEY_H_INCLUDED
#define PUBLICKEY_H_INCLUDED

#include <QString>

class PublicKeyData;
class PrivateKey;
class TLS;

/**
 * Wraps around an OpenSSL public key.
 *
 * This class has conversion operators to PrivateKey which let the user fetch
 * the private key from the key pair if (and only if) the data is available.
 *
 * The class uses implicit data sharing and copy-on-write to ensure good
 * performance when passing keys by value.
 *
 * It is possible for a PublicKey to be invalid if it does not contain any key
 * information, e.g. after the default constructor has been called. In this case
 * isValid() returns false.
 */
class PublicKey {
public:
	/**
	 * Default constructor.
	 *
	 * Creates an invalid key.
	 */
	PublicKey();
	/**
	 * Copy constructor.
	 */
	PublicKey(const PublicKey &other);
	/**
	 * Constructor.
	 *
	 * Creates a public key from the private key. This always succeeds, in
	 * contrast to conversions from a public to a private key.
	 */
	PublicKey(const PrivateKey &other);
	/**
	 * Destructor.
	 */
	~PublicKey();

	/**
	 * Creates a public key from  the PEM text representation.
	 *
	 * @param data PEM key data.
	 * @return Public key holding the data, or an invalid key if the passed data
	 * is invalid.
	 */
	static PublicKey fromPEM(QString data);
	/**
	 * Creates a public key from binary DER data.
	 *
	 * @param data Binary DER key data.
	 * @return Public key holding the data, or an invalid key if the passed data
	 * is invalid.
	 */
	static PublicKey fromDER(QByteArray data);
	/**
	 * Creates the PEM representation from the key.
	 *
	 * @return PEM representation.
	 */
	QString toPEM() const;
	/**
	 * Creates the DER binary representation from the key.
	 *
	 * @return DER representation.
	 */
	QByteArray toDER() const;

	/**
	 * Creates an SHA-1 fingerprint of the public key.
	 *
	 * @return SHA-1 fingerprint.
	 */
	QString fingerprint() const;
	/**
	 * Gets a 32-bit hash value from the key data.
	 *
	 * Useful if you want to use the key as the key to a QMap or similar.
	 *
	 * @return Hash value.
	 */
	unsigned int hash() const;

	/**
	 * Saves the key PEM-encoded.
	 *
	 * @param fileName File to save the key to.
	 * @return True if the key could be saved.
	 */
	bool save(QString fileName) const;
	/**
	 * Loads the key from a file containing the PEM-encoded key data.
	 *
	 * @param fileName File to load the key from.
	 * @return Loaded key or an invalid key if the key could not be loaded.
	 */
	static PublicKey load(QString fileName);

	/**
	 * Returns whether this key object is valid.
	 *
	 * @return False if the PublicKey does not contain any valid key.
	 */
	bool isValid() const {
		return keyData != NULL;
	}

	/**
	 * Verifies the signature for the data and this public key.
	 * @param data Data which has been signed.
	 * @param signature Signature belonging to the data and issued by the
	 * private key belonging to this key.
	 * @return True if the signature could be verified, false if it is
	 * corrupt.
	 */
	bool verify(QByteArray data, QByteArray signature);

	/**
	 * Assignment operator.
	 */
	PublicKey &operator=(const PublicKey &other);
	/**
	 * Assignment operator to convert a private key into a public key.
	 */
	PublicKey &operator=(const PrivateKey &other);
	/**
	 * Checks whether the two keys are identical.
	 */
	bool operator==(const PublicKey &other) const;
	/**
	 * Checks whether the two keys are identical.
	 *
	 * This works as if you converted the private key to a public key and
	 * compared afterwards.
	 */
	bool operator==(const PrivateKey &other) const;
private:
	PublicKey(PublicKeyData *keyData);

	/**
	 * Implicitly shared key data.
	 */
	PublicKeyData *keyData;

	friend class PrivateKey;
	friend class Certificate;
	friend class TLS;
};

/**
 * Wraps around an OpenSSL public key.
 *
 * This class has conversion operators to PublicKey which let the user fetch
 * the public key from the key pair.
 *
 * The class uses implicit data sharing and copy-on-write to ensure good
 * performance when passing keys by value.
 *
 * It is possible for a PrivateKey to be invalid if it does not contain any key
 * information, e.g. after the default constructor has been called. In this case
 * isValid() returns false.
 */
class PrivateKey {
public:
	/**
	 * Default constructor.
	 *
	 * Creates an invalid key.
	 */
	PrivateKey();
	/**
	 * Copy constructor.
	 */
	PrivateKey(const PrivateKey &other);
	/**
	 * Constructor.
	 *
	 * Creates a private key from the public key. This failes and leaves the
	 * key invalid if the other key does not already contain a private key
	 * internally.
	 */
	PrivateKey(const PublicKey &other);
	/**
	 * Destructor.
	 */
	~PrivateKey();

	/**
	 * Creates a new random key pair.
	 *
	 * @param bits Number of bits of the new key pair.
	 * @return Generated private key.
	 */
	static PrivateKey generate(unsigned int bits = 2048);

	/**
	 * Creates a private key from  the PEM text representation.
	 *
	 * @param data PEM key data.
	 * @return Private key holding the data, or an invalid key if the passed data
	 * is invalid.
	 */
	static PrivateKey fromPEM(QString data);
	/**
	 * Creates a private key from binary DER data.
	 *
	 * @param data Binary DER key data.
	 * @return Private key holding the data, or an invalid key if the passed
	 * data is invalid.
	 */
	static PrivateKey fromDER(QByteArray data);
	/**
	 * Creates the PEM representation from the key.
	 *
	 * @return PEM representation.
	 */
	QString toPEM() const;
	/**
	 * Creates the DER binary representation from the key.
	 *
	 * @return DER representation.
	 */
	QByteArray toDER() const;

	/**
	 * Saves the key PEM-encoded.
	 *
	 * @param fileName File to save the key to.
	 * @return True if the key could be saved.
	 */
	bool save(QString fileName) const;
	/**
	 * Loads the key from a file containing the PEM-encoded key data.
	 *
	 * @param fileName File to load the key from.
	 * @return Loaded key or an invalid key if the key could not be loaded.
	 */
	static PrivateKey load(QString fileName);

	/**
	 * Returns whether this key object is valid.
	 *
	 * @return False if the PrivateKey does not contain any valid key.
	 */
	bool isValid() const {
		return keyData != NULL;
	}

	/**
	 * Signs the data using this private key so that it lated can be verified with
	 * PublicKey::verify().
	 * @param data Data to be signed.
	 * @return Signature.
	 */
	QByteArray sign(QByteArray data);

	/**
	 * Assignment operator.
	 */
	PrivateKey &operator=(const PrivateKey &other);
	/**
	 * Assignment operator to create a private key from a public key.
	 *
	 * This returns an invalid key if the public key does not already contain
	 * private key data.
	 */
	PrivateKey &operator=(const PublicKey &other);
	/**
	 * Checks whether the two keys are identical.
	 */
	bool operator==(const PrivateKey &other) const;
	/**
	 * Checks whether the two keys are identical.
	 *
	 * This works as if you converted the private key to a public key and
	 * compared afterwards.
	 */
	bool operator==(const PublicKey &other) const;
private:
	PrivateKey(PublicKeyData *keyData);

	/**
	 * Implicitly shared key data.
	 */
	PublicKeyData *keyData;

	friend class PublicKey;
	friend class TLS;
};

#endif
