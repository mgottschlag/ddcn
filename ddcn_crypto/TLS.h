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

#ifndef TLS_H_INCLUDED
#define TLS_H_INCLUDED

#include "Certificate.h"
#include "PublicKey.h"

#include <QObject>
#include <openssl/ssl.h>
struct AcceptConnectState {
	enum List {
		Finished,
		Error,
		TryAgain
	};
};

class TLS : public QObject {
	Q_OBJECT
public:
	static void initialize();

	TLS();
	~TLS();
	bool startServer(/* TODO */);
	bool startClient(/* TODO */);

	void setCertificate(Certificate cert);
	Certificate getCertificate();
	void setPrivateKey(PrivateKey key);
	PrivateKey getPrivateKey();

	void write(const QByteArray &data);
	QByteArray read();

	Certificate getPeerCertificate();

	void writeIncoming(const QByteArray &data);
	QByteArray readOutgoing();
signals:
	void handshakeComplete();
	void readyRead();
	void readyReadOutgoing();
	void error();
private:
	bool init(const SSL_METHOD *method);

	struct AcceptConnectState {
		enum List {
			Finished,
			Error,
			TryAgain
		};
	};

	AcceptConnectState::List doHandshake();
	AcceptConnectState::List nonBlockigConnectAccept(int status);
	bool doRead();
	bool doWrite();

	Certificate cert;
	Certificate peerCert;
	PrivateKey key;

	SSL_CTX *context;
	SSL *ssl;
	BIO *rbio;
	BIO *wbio;

	bool server;
	bool handshaken;

	QByteArray toNetwork;
	QByteArray readBuffer;
	QByteArray writeBuffer;
};

#endif
