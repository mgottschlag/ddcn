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

#ifndef CERTIFICATE_H_INCLUDED
#define CERTIFICATE_H_INCLUDED

#include "PublicKey.h"

class CertificateData;
class TLS;

/**
 * Wraps around an OpenSSL certificate.
 *
 * This class only implements the absolutely necessary functionality as
 * certificates are not used anywhere.
 */
class Certificate {
public:
	/**
	 * Constructor. Creates an invalid certificate.
	 */
	Certificate();
	/**
	 * Copy constructor.
	 */
	Certificate(const Certificate &cert);
	/**
	 * Destructor.
	 */
	~Certificate();

	/**
	 * Creates a self-signed certificate from the given public key.
	 *
	 * The public key has to contain valid private key information.
	 *
	 * @param key Public key to be used for the certificate.
	 * @return Certificate, might be invalid if the key is not also a valid
	 * private key.
	 */
	static Certificate createSelfSigned(const PublicKey &key);

	/**
	 * Returns the public key of the certificate.
	 *
	 * @return Public key of the certificarte.
	 */
	PublicKey getPublicKey();

	/**
	 * Returns whether the certificate is valid.
	 */
	bool isValid();

	/**
	 * Assignment operator.
	 *
	 * This returns an invalid certificate if and only if cert is invalid.
	 */
	Certificate &operator=(const Certificate &cert);
private:
	Certificate(CertificateData *certData);

	CertificateData *certData;

	friend class TLS;
};

#endif
