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

#ifndef PUBLICKEYDATA_H_INCLUDED
#define PUBLICKEYDATA_H_INCLUDED

#include <openssl/evp.h>
#include <QString>

struct PublicKeyData {
public:
	PublicKeyData() : refCount(0), key(NULL), privateKey(false), hasHash(false) {
	}
	~PublicKeyData() {
		if (key) {
			EVP_PKEY_free(key);
		}
	}

	/**
	 * Increases the reference count for this key data object.
	 * @note This is not thread-safe.
	 */
	void grab() {
		refCount++;
	}
	/**
	 * Decreases the reference count for this key data object.
	 *
	 * This deletes the object if the reference count goes down to 0.
	 * @note This is not thread-safe.
	 */
	void drop() {
		if (--refCount == 0) {
			delete this;
		}
	}

	void setKey(EVP_PKEY *key) {
		// TODO: Remove this, only allow setting in the constructor
		this->key = key;
	}
	EVP_PKEY *getKey() {
		return key;
	}
	QString getFingerprint();
	unsigned int getHash();

	void setPrivateKey(bool privateKey) {
		this->privateKey = privateKey;
	}
	bool isPrivateKey() {
		return privateKey;
	}
private:
	int refCount;
	EVP_PKEY *key;
	bool privateKey;

	QString fingerprint;
	bool hasHash;
	unsigned int hash;
};

#endif
