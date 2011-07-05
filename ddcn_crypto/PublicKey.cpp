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
EVENT SHALL <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PublicKey.h"
#include "PublicKeyData.h"

#include <openssl/pem.h>
#include <openssl/evp.h>
#include <QFile>

static QByteArray toByteArray(BIO *bio) {
	QByteArray buffer;
	bool atEnd = false;
	QByteArray tmp;
	tmp.resize(4096);
	while (!atEnd) {
		int status = BIO_read(bio, tmp.data(), tmp.size());
		if (status > 0) {
			buffer.append(tmp.left(status));
		} else {
			atEnd = true;
		}
	}
	BIO_free(bio);
	return buffer;
}

PublicKey::PublicKey() : keyData(NULL) {
}
PublicKey::PublicKey(const PublicKey &other) : keyData(other.keyData) {
	if (keyData) {
		keyData->grab();
	}
}
PublicKey::PublicKey(const PrivateKey &other) : keyData(other.keyData) {
	if (keyData) {
		keyData->grab();
	}
}
PublicKey::~PublicKey() {
	if (keyData) {
		keyData->drop();
	}
}

PublicKey PublicKey::fromPEM(QString data) {
	BIO *bio = BIO_new(BIO_s_mem());
	QByteArray ascii = data.toAscii();
	BIO_write(bio, ascii.data(), ascii.size());
	// We do not support encrypting the key with a password
	EVP_PKEY *osslKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
	BIO_free(bio);
	if (!osslKey) {
		return PublicKey();
	}
	PublicKeyData *keyData = new PublicKeyData();
	keyData->setKey(osslKey);
	PublicKey key(keyData);
	return key;
}
PublicKey PublicKey::fromDER(QByteArray data) {
	BIO *bio = BIO_new(BIO_s_mem());
	BIO_write(bio, data.data(), data.size());
	// We do not support encrypting the key with a password
	EVP_PKEY *osslKey = d2i_PUBKEY_bio(bio, NULL);
	BIO_free(bio);
	if (!osslKey) {
		return PublicKey();
	}
	PublicKeyData *keyData = new PublicKeyData();
	keyData->setKey(osslKey);
	PublicKey key(keyData);
	return key;
}
QString PublicKey::toPEM() const {
	if (!isValid()) {
		return "";
	}
	EVP_PKEY *osslKey = keyData->getKey();
	BIO *bio = BIO_new(BIO_s_mem());
	PEM_write_bio_PUBKEY(bio, osslKey);
	QByteArray buf = toByteArray(bio);
	return QString::fromAscii(buf);
}
QByteArray PublicKey::toDER() const {
	if (!isValid()) {
		return "";
	}
	EVP_PKEY *osslKey = keyData->getKey();
	BIO *bio = BIO_new(BIO_s_mem());
	i2d_PUBKEY_bio(bio, osslKey);
	return toByteArray(bio);
}

QString PublicKey::fingerprint() const {
	if (!isValid()) {
		return "";
	}
	return keyData->getFingerprint();
}
unsigned int PublicKey::hash() const {
	if (!isValid()) {
		return 0;
	}
	return keyData->getHash();
}

bool PublicKey::save(QString fileName) const {
	// TODO: Write directly?
	QString pem = toPEM();
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	file.write(pem.toAscii());
	return true;
}
PublicKey PublicKey::load(QString fileName) {
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return PublicKey();
	}
	QByteArray pem = file.readAll();
	return fromPEM(QString::fromAscii(pem));
}

PublicKey &PublicKey::operator=(const PublicKey &other) {
	if (&other == this) {
		return *this;
	}
	if (keyData) {
		keyData->drop();
	}
	keyData = other.keyData;
	if (keyData) {
		keyData->grab();
	}
	return *this;
}
PublicKey &PublicKey::operator=(const PrivateKey &other) {
	if (keyData) {
		keyData->drop();
	}
	keyData = other.keyData;
	if (keyData) {
		keyData->grab();
	}
	return *this;
}
bool PublicKey::operator==(const PublicKey &other) const {
	if (keyData == other.keyData) {
		return true;
	}
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return EVP_PKEY_cmp(keyData->getKey(), other.keyData->getKey()) == 1;
}
bool PublicKey::operator==(const PrivateKey &other) const {
	if (keyData == other.keyData) {
		return true;
	}
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return EVP_PKEY_cmp(keyData->getKey(), other.keyData->getKey()) == 1;
}

PublicKey::PublicKey(PublicKeyData *keyData) : keyData(keyData) {
	if (keyData) {
		keyData->grab();
	}
}

PrivateKey::PrivateKey() : keyData(NULL) {
}
PrivateKey::PrivateKey(const PrivateKey &other) : keyData(other.keyData) {
	if (keyData) {
		keyData->grab();
	}
}
PrivateKey::PrivateKey(const PublicKey &other) : keyData(other.keyData) {
	if (keyData) {
		if (keyData->isPrivateKey()) {
			keyData->grab();
		} else {
			keyData = NULL;
		}
	}
}PrivateKey::~PrivateKey() {
	if (keyData) {
		keyData->drop();
	}
}

PrivateKey PrivateKey::generate(unsigned int bits) {
	RSA *rsa = RSA_generate_key(bits, RSA_F4, NULL, NULL);
	EVP_PKEY *pkey = EVP_PKEY_new();
	if (EVP_PKEY_assign_RSA(pkey, rsa) == 0) {
		qFatal("Could not assign RSA key.");
	}
	PublicKeyData *keyData = new PublicKeyData();
	keyData->setPrivateKey(true);
	keyData->setKey(pkey);
	PrivateKey key(keyData);
	return key;
}

PrivateKey PrivateKey::fromPEM(QString data) {
	BIO *bio = BIO_new(BIO_s_mem());
	QByteArray ascii = data.toAscii();
	BIO_write(bio, ascii.data(), ascii.size());
	// We do not support encrypting the key with a password
	EVP_PKEY *osslKey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
	BIO_free(bio);
	if (!osslKey) {
		return PrivateKey();
	}
	PublicKeyData *keyData = new PublicKeyData();
	keyData->setPrivateKey(true);
	keyData->setKey(osslKey);
	PrivateKey key(keyData);
	return key;
}
PrivateKey PrivateKey::fromDER(QByteArray data) {
	BIO *bio = BIO_new(BIO_s_mem());
	BIO_write(bio, data.data(), data.size());
	// We do not support encrypting the key with a password
	EVP_PKEY *osslKey = d2i_PrivateKey_bio(bio, NULL);
	BIO_free(bio);
	if (!osslKey) {
		return PrivateKey();
	}
	PublicKeyData *keyData = new PublicKeyData();
	keyData->setPrivateKey(true);
	keyData->setKey(osslKey);
	PrivateKey key(keyData);
	return key;
}
QString PrivateKey::toPEM() const {
	if (!isValid()) {
		return "";
	}
	EVP_PKEY *osslKey = keyData->getKey();
	BIO *bio = BIO_new(BIO_s_mem());
	PEM_write_bio_PrivateKey(bio, osslKey, NULL, NULL, 0, NULL, NULL);
	QByteArray buf = toByteArray(bio);
	return QString::fromAscii(buf);
}
QByteArray PrivateKey::toDER() const {
	if (!isValid()) {
		return "";
	}
	EVP_PKEY *osslKey = keyData->getKey();
	BIO *bio = BIO_new(BIO_s_mem());
	i2d_PrivateKey_bio(bio, osslKey);
	return toByteArray(bio);
}

bool PrivateKey::save(QString fileName) const {
	// TODO: Write directly?
	QString pem = toPEM();
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	file.write(pem.toAscii());
	return true;
}
PrivateKey PrivateKey::load(QString fileName) {
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return PublicKey();
	}
	QByteArray pem = file.readAll();
	return fromPEM(QString::fromAscii(pem.data()));

}

PrivateKey &PrivateKey::operator=(const PrivateKey &other) {
	if (&other == this) {
		return *this;
	}
	if (keyData) {
		keyData->drop();
	}
	keyData = other.keyData;
	if (keyData) {
		keyData->grab();
	}
	return *this;
}
PrivateKey &PrivateKey::operator=(const PublicKey &other) {
	if (!other.isValid() || !other.keyData->isPrivateKey()) {
		*this = PrivateKey();
		return *this;
	}
	if (keyData) {
		keyData->drop();
	}
	keyData = other.keyData;
	if (keyData) {
		keyData->grab();
	}
	return *this;
}
bool PrivateKey::operator==(const PrivateKey &other) const {
	if (keyData == other.keyData) {
		return true;
	}
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return EVP_PKEY_cmp(keyData->getKey(), other.keyData->getKey()) == 1;
}
bool PrivateKey::operator==(const PublicKey &other) const {
	if (keyData == other.keyData) {
		return true;
	}
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return EVP_PKEY_cmp(keyData->getKey(), other.keyData->getKey()) == 1;
}

PrivateKey::PrivateKey(PublicKeyData *keyData) : keyData(keyData) {
	if (keyData) {
		keyData->grab();
	}
}
