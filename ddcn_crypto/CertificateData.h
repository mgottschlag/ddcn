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

#ifndef CERTIFICATEDATA_H_INCLUDED
#define CERTIFICATEDATA_H_INCLUDED

#include "PublicKey.h"

#include <openssl/evp.h>
#include <openssl/x509.h>

struct CertificateData {
public:
	CertificateData(X509 *cert) : refCount(0), cert(cert) {
	}
	~CertificateData() {
		if (cert) {
			X509_free(cert);
		}
	}

	void grab() {
		refCount++;
	}
	void drop() {
		if (--refCount == 0) {
			delete this;
		}
	}

	X509 *getCert() {
		return cert;
	}

	void setPublicKey(const PublicKey &key) {
		this->key = key;
	}
	PublicKey getPublicKey() {
		return key;
	}
private:
	int refCount;
	X509 *cert;

	PublicKey key;
};

#endif
