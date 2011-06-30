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

#include "Certificate.h"
#include "CertificateData.h"
#include "PublicKeyData.h"

Certificate::Certificate() : certData(NULL) {
}
Certificate::Certificate(const Certificate &cert) : certData(cert.certData) {
	if (certData) {
		certData->grab();
	}
}
Certificate::~Certificate() {
	if (certData) {
		certData->drop();
	}
}

Certificate Certificate::createSelfSigned(const PublicKey &key) {
	if (!key.isValid()) {
		return Certificate();
	}
	PublicKeyData *keyData = key.keyData;
	EVP_PKEY *osslKey = keyData->getKey();
	X509 *osslCert = X509_new();
	X509_set_version(osslCert, 3);
	ASN1_INTEGER_set(X509_get_serialNumber(osslCert), 0);
	X509_gmtime_adj(X509_get_notBefore(osslCert), 0);
	X509_gmtime_adj(X509_get_notAfter(osslCert), (long)60*60*24*365);
	X509_set_pubkey(osslCert, osslKey);
	CertificateData *certData = new CertificateData();
	certData->setCert(osslCert);
	certData->setPublicKey(key);
	Certificate certificate(certData);
	return certificate;
}

PublicKey Certificate::getPublicKey() {
	if (!isValid()) {
		return PublicKey();
	}
	if (certData->getPublicKey().isValid()) {
		return certData->getPublicKey();
	}
	// Extract the public key from the X509 certificate
	EVP_PKEY *osslKey = X509_get_pubkey(certData->getCert());
	PublicKeyData *keyData = new PublicKeyData();
	keyData->setKey(osslKey);
	PublicKey key(keyData);
	certData->setPublicKey(key);
	return key;
}

bool Certificate::isValid() {
	return certData != NULL;
}

Certificate &Certificate::operator=(const Certificate &cert) {
	if (this == &cert) {
		return *this;
	}
	if (certData) {
		certData->drop();
	}
	certData = cert.certData;
	if (certData) {
		certData->grab();
	}
	return *this;
}

Certificate::Certificate(CertificateData *certData) {
	certData->grab();
	this->certData = certData;
}
