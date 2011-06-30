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

#ifndef PUBLICKEY_H_INCLUDED
#define PUBLICKEY_H_INCLUDED

#include <QString>

class PublicKeyData;
class PrivateKey;
class TLS;

class PublicKey {
public:
	PublicKey();
	PublicKey(const PublicKey &other);
	PublicKey(const PrivateKey &other);
	~PublicKey();

	static PublicKey fromPEM(QString data);
	static PublicKey fromDER(QByteArray data);
	QString toPEM() const;
	QByteArray toDER() const;

	QString fingerprint() const;
	unsigned int hash() const;

	bool save(QString fileName) const;
	static PublicKey load(QString fileName);

	bool isValid() const {
		return keyData != NULL;
	}

	PublicKey &operator=(const PublicKey &other);
	PublicKey &operator=(const PrivateKey &other);
	bool operator==(const PublicKey &other) const;
	bool operator==(const PrivateKey &other) const;
private:
	PublicKey(PublicKeyData *keyData);

	PublicKeyData *keyData;

	friend class PrivateKey;
	friend class Certificate;
	friend class TLS;
};

class PrivateKey {
public:
	PrivateKey();
	PrivateKey(const PrivateKey &other);
	PrivateKey(const PublicKey &other);
	~PrivateKey();

	static PrivateKey generate(unsigned int bits = 2048);

	static PublicKey fromPEM(QString data);
	static PublicKey fromDER(QByteArray data);
	QString toPEM() const;
	QByteArray toDER() const;

	bool save(QString fileName) const;
	static PublicKey load(QString fileName);

	bool isValid() const {
		return keyData != NULL;
	}

	PrivateKey &operator=(const PrivateKey &other);
	PrivateKey &operator=(const PublicKey &other);
	bool operator==(const PrivateKey &other) const;
	bool operator==(const PublicKey &other) const;
private:
	PrivateKey(PublicKeyData *keyData);

	PublicKeyData *keyData;

	friend class PublicKey;
	friend class TLS;
};

#endif
