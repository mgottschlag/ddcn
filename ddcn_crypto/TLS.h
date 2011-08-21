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

#ifndef TLS_H_INCLUDED
#define TLS_H_INCLUDED

#include "Certificate.h"
#include "PublicKey.h"

#include <QObject>
#include <openssl/ssl.h>

/**
 * @mainpage ddcn_crypto
 *
 * @section about About
 *
 * These classes implement a simple Qt-like wrapper around OpenSSL to be able
 * to easily create TLS connections. The reason why not one of the existing C++
 * SSL libraries were used is that none featured both client authentication and
 * non-blocking input/output.
 *
 * @section structure Structure
 *
 * The main class, TLS, wraps around a simple TLS connection. There are helper
 * classes, PublicKey, PrivateKey and Certificate, which store the keys and
 * certificates needed in that class.
 *
 * These helper classes implement implicit data sharing which means that they
 * can easily be copied around very often without much performance penalty.
 */

/**
 * Encapsulates a TLS connection with both client and server authentication.
 *
 * We use a full TLS handshake to identicate the other peer as well as we
 * have symmetric connections.
 */
class TLS : public QObject {
	Q_OBJECT
public:
	/**
	 * Initializes OpenSSL.
	 *
	 * Has to be called befor any other ddcn_crypto class is used.
	 */
	static void initialize();

	/**
	 * Constructor.
	 */
	TLS();
	/**
	 * Destructor.
	 */
	~TLS();
	/**
	 * Starts this connection in server mode.
	 *
	 * @return False if an error occurred.
	 */
	bool startServer();
	/**
	 * Starts this connection in client mode.
	 *
	 * @return False if an error occurred.
	 */
	bool startClient();

	/**
	 * Sets the certificate of the local peer. Has to be called before the
	 * connection is started.
	 *
	 * @param cert Certificate created with the local key.
	 */
	void setCertificate(Certificate cert);
	/**
	 * Returns the certificate of the local peer.
	 *
	 * @return Local certificate.
	 */
	Certificate getCertificate();
	/**
	 * Sets the private key of this peer. Has to be called before the connection
	 * is started.
	 *
	 * @param key Private key of this peer.
	 */
	void setPrivateKey(PrivateKey key);
	/**
	 * Returns the private key of this peer.
	 *
	 * @return Private key of this peer.
	 */
	PrivateKey getPrivateKey();

	/**
	 * Writes some (non-encrypted) data to this connection. This might trigger
	 * one or more readyReadOutgoing() signals when OpenSSL encrypts the data.
	 *
	 * @param data Data to be written to the TLS stream.
	 */
	void write(const QByteArray &data);
	/**
	 * Reads some (non-encrypted) data from this connection. This function can
	 * be called and will return a non-empty array only after readyRead() has
	 * been triggered.
	 *
	 * @return Array with all data received since the last call to this
	 * function.
	 */
	QByteArray read();

	/**
	 * Returns the certificate of the other peer.
	 *
	 * This can be used to check the public key of the peer. The certificate
	 * might be invalid if the other peer tried to connect without client
	 * authentication.
	 *
	 * @return Certificate of the other peer.
	 */
	Certificate getPeerCertificate();

	/**
	 * Writes some (encrypted) data which has been received from the network
	 * into this TLS stream. This might trigger one or more readyRead() signals.
	 *
	 * @param data Encrypted data.
	 */
	void writeIncoming(const QByteArray &data);
	/**
	 * Reads all (encrypted) data which shall be sent over the network.This
	 * returns data only after readyReadOutgoing() has been triggered.
	 *
	 * @return Data to be sent over the network.
	 */
	QByteArray readOutgoing();
signals:
	/**
	 * Triggered after the handshake is complete and the connection is ready.
	 */
	void handshakeComplete();
	/**
	 * Triggered when unencrypted data is available for this peer to read.
	 */
	void readyRead();
	/**
	 * Triggered when encrypted data is ready to be send over the network.
	 */
	void readyReadOutgoing();
	/**
	 * Triggered when an error occurred and the connection has to be reset.
	 */
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
	AcceptConnectState::List nonBlockingConnectAccept(int status);
	bool doRead();
	bool doWrite();
	bool checkOutgoing();
	void continueHandshake();

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
