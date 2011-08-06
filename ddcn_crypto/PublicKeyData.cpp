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

#include "PublicKeyData.h"

#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <QHash>

QByteArray toByteArray(BIGNUM *bignum) {
	QByteArray data;
	data.resize(BN_num_bytes(bignum));
	BN_bn2bin(bignum, (unsigned char*)data.data());
	return data;
}

QString PublicKeyData::getFingerprint() {
	// Use precomputed fingerprint if possible
	if (fingerprint != "") {
		return fingerprint;
	}
	EVP_MD_CTX context;
	const EVP_MD *method = EVP_sha1();
	// Get data to be hashed
	QByteArray keyData;
	switch (EVP_PKEY_type(key->type)) {
#ifndef OPENSSL_NO_RSA
		case EVP_PKEY_RSA:
			keyData.append(toByteArray(key->pkey.rsa->n));
			keyData.append(toByteArray(key->pkey.rsa->e));
			break;
#endif
#ifndef OPENSSL_NO_DSA
		case EVP_PKEY_DSA:
			// TODO: Not compatible to OpenSSH fingerprint
			keyData.append(toByteArray(key->pkey.dsa->p));
			keyData.append(toByteArray(key->pkey.dsa->q));
			keyData.append(toByteArray(key->pkey.dsa->g));
			keyData.append(toByteArray(key->pkey.dsa->pub_key));
			break;
#endif
		default:
			// TODO: Are there other key types which need to be added?
			return "";
	}
	// Hash data
	QByteArray fingerprint;
	fingerprint.resize(EVP_MAX_MD_SIZE);
	EVP_DigestInit(&context, method);
	EVP_DigestUpdate(&context, keyData.data(), keyData.size());
	uint digestSize = 0;
	EVP_DigestFinal(&context, (unsigned char*)fingerprint.data(), &digestSize);
	this->fingerprint = QString::fromAscii(fingerprint.left(digestSize).toHex());
	return this->fingerprint;
}

unsigned int PublicKeyData::getHash() {
	if (hasHash) {
		return hash;
	}
	QString fingerprint = getFingerprint();
	hash = qHash(fingerprint);
	hasHash = true;
	return hash;
}
