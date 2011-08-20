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

#include "TLS.h"
#include "PublicKeyData.h"
#include "CertificateData.h"

#include <openssl/err.h>
#include <stdio.h>

void TLS::initialize() {
	// Initialize OpenSSL
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
}

TLS::TLS() : context(NULL), ssl(NULL), handshaken(false) {
}
TLS::~TLS() {
	if (context) {
		SSL_CTX_free(context);
	}
	if (ssl) {
		SSL_free(ssl);
	}
}
bool TLS::startServer() {
	server = true;
	if (!cert.isValid()) {
		qFatal("No valid cert!");
	}
	if (!key.isValid()) {
		qFatal("No valid private key!");
	}
	const SSL_METHOD *method = TLSv1_server_method();
	if (!init(method)) {
		qFatal("Could not initialize TLS context.");
	}
	SSL_set_bio(ssl, rbio, wbio);
	AcceptConnectState::List handshakeState = doHandshake();
	if (handshakeState == AcceptConnectState::Error) {
		return false;
	} else if (handshakeState == AcceptConnectState::Finished) {
		emit handshakeComplete();
	}
	checkOutgoing();
	return true;
}
bool TLS::startClient() {
	server = false;
	if (!cert.isValid()) {
		qFatal("No valid cert!");
	}
	if (!key.isValid()) {
		qFatal("No valid private key!");
	}
	const SSL_METHOD *method = TLSv1_client_method();
	if (!init(method)) {
		qFatal("Could not initialize TLS context.");
	}
	SSL_set_bio(ssl, rbio, wbio);
	AcceptConnectState::List handshakeState = doHandshake();
	if (handshakeState == AcceptConnectState::Error) {
		return false;
	} else if (handshakeState == AcceptConnectState::Finished) {
		emit handshakeComplete();
	}
	checkOutgoing();
	return true;
}

void TLS::setCertificate(Certificate cert) {
	this->cert = cert;
}
Certificate TLS::getCertificate() {
	return cert;
}
void TLS::setPrivateKey(PrivateKey key) {
	this->key = key;
}
PrivateKey TLS::getPrivateKey() {
	return key;
}

void TLS::write(const QByteArray &data) {
	if (!handshaken) {
		qFatal("write() called without handshake done.");
	}
	writeBuffer.append(data);
	doWrite();
	checkOutgoing();
}
QByteArray TLS::read() {
	if (!handshaken) {
		qFatal("read() called without handshake done.");
	}
	doRead();
	QByteArray buffer = readBuffer;
	readBuffer.clear();
	return buffer;
}

Certificate TLS::getPeerCertificate() {
	if (!handshaken) {
		qFatal("getPeerCertificate() called without handshake done.");
	}
	return peerCert;
}

void TLS::writeIncoming(const QByteArray &data) {
	BIO_write(rbio, data.data(), data.size());
	if (!handshaken) {
		continueHandshake();
	}
	if (handshaken) {
		doRead();
		if (readBuffer.size() > 0) {
			emit readyRead();
		}
	}
}
QByteArray TLS::readOutgoing() {
	QByteArray data = toNetwork;
	toNetwork.clear();
	return data;
}

static int tlsVerifyCallback(X509_STORE_CTX *context, void *userdata) {
	// The only thing we need to check is that the public key of this peer is
	// valid, and this is done in NetworkNode
	return 1;
}

bool TLS::init(const SSL_METHOD *method) {
	// Create context with key/cert
	context = SSL_CTX_new((SSL_METHOD*)method);
	if (!context) {
		ERR_print_errors_fp(stderr);
		return false;
	}
	if (!SSL_CTX_use_PrivateKey(context, key.keyData->getKey())) {
		ERR_print_errors_fp(stderr);
		return false;
	}
	if (!SSL_CTX_use_certificate(context, cert.certData->getCert())) {
		ERR_print_errors_fp(stderr);
		return false;
	}
	if (!SSL_CTX_check_private_key(context)) {
		qCritical("Private key does not match the certificate public key.");
		return false;
	}
	// Enable client authentication
	SSL_CTX_set_verify(context, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
	// Do not actually check the certificate, we only need the public key
	SSL_CTX_set_cert_verify_callback(context, tlsVerifyCallback, NULL);
	SSL_CTX_set_mode(context, SSL_MODE_ENABLE_PARTIAL_WRITE);
	// Create SSL instance
	ssl = SSL_new(context);
	if (!ssl) {
		ERR_print_errors_fp(stderr);
		return false;
	}
	// Set BIOs to plug in our custom network layer
	rbio = BIO_new(BIO_s_mem());
	wbio = BIO_new(BIO_s_mem());
	// Note: OpenSSL takes over the BIO ownership, no need to free them later
	SSL_set_bio(ssl, rbio, wbio);
	handshaken = false;
	return true;
}

TLS::AcceptConnectState::List TLS::doHandshake() {
	int status;
	if (server) {
		status = SSL_accept(ssl);
	} else {
		status = SSL_connect(ssl);
	}
	return nonBlockingConnectAccept(status);
}

TLS::AcceptConnectState::List TLS::nonBlockingConnectAccept(int status) {
	if (status <= 0) {
		int error = SSL_get_error(ssl, status);
		if (error == SSL_ERROR_WANT_CONNECT || error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
			return AcceptConnectState::TryAgain;
		} else {
			qDebug("Error while doing handshake: %s", ERR_error_string(ERR_get_error(), NULL));
			return AcceptConnectState::Error;
		}
	} else {
		handshaken = true;
		// Fetch remote certificate
		CertificateData *certData = new CertificateData(SSL_get_peer_certificate(ssl));
		peerCert = Certificate(certData);
		return AcceptConnectState::Finished;
	}
}

bool TLS::doRead() {
	bool atEnd = false;
	QByteArray buffer;
	buffer.resize(8192);
	while (!atEnd) {
		int status = SSL_read(ssl, buffer.data(), buffer.size());
		if (status > 0) {
			readBuffer.append(buffer.left(status));
		} else {
			int errorCode = SSL_get_error(ssl, status);
			if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE) {
				break;
			} else if(errorCode == SSL_ERROR_ZERO_RETURN) {
				atEnd = true;
			} else {
				return false;
			}
		}
	}
	return true;
}
bool TLS::doWrite() {
	if (writeBuffer.size() > 0) {
		int status = SSL_write(ssl, writeBuffer.data(), writeBuffer.size());
		if (status > 0) {
			// Parts of the data were sent
			writeBuffer = writeBuffer.right(writeBuffer.size() - status);
		} else {
			int errorCode = SSL_get_error(ssl, status);
			if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE) {
				return true;
			} else {
				return false;
			}
		}
	}
	return true;
}
bool TLS::checkOutgoing() {
	int size = BIO_pending(wbio);
	if (size <= 0) {
		return false;
	}
	QByteArray outgoing;
	outgoing.resize(size);
	BIO_read(wbio, outgoing.data(), outgoing.size());
	toNetwork.append(outgoing);
	emit readyReadOutgoing();
	if (outgoing.size() > 0) {
		doWrite();
		checkOutgoing();
		return true;
	} else {
		return false;
	}
}

void TLS::continueHandshake() {
	AcceptConnectState::List handshakeState = doHandshake();
	if (handshakeState == AcceptConnectState::Error) {
		emit error();
		return;
	} else if (handshakeState == AcceptConnectState::Finished) {
		emit handshakeComplete();
	}
	if (checkOutgoing() && !handshaken) {
		// The handshake might end with this peer sending data, in which case
		// we have to check again whether the handshake is finished
		continueHandshake();
	}
}
