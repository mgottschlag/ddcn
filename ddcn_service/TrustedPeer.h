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

#ifndef TRUSTEDPEER_H_INCLUDED
#define TRUSTEDPEER_H_INCLUDED

#include "PublicKey.h"

#include <QString>

class NetworkNode;

/**
 * Stores the information about a trusted peer.
 *
 * CompilerNetwork holds a list of these and reads/saves them from a file
 */
class TrustedPeer {
public:
	/**
	 * Constructor. Called from CompilerNetwork::addTrustedPeer().
	 */
	TrustedPeer(QString name, const PublicKey &publicKey) : name(name),
			publicKey(publicKey), node(NULL) {
	}

	/**
	 * Sets the name of the trusted peer. This name is purely descriptive and
	 * can be freely set by the service.
	 */
	void setName(QString name) {
		this->name = name;
	}
	/**
	 * Returns the name of the trusted peer.
	 */
	QString getName() {
		return name;
	}
	/**
	 * Returns the public key identifying the trusted peer.
	 */
	PublicKey getPublicKey() {
		return publicKey;
	}

	/**
	 * Sets the network node of this trusted peer when we have connected to it.
	 */
	void setNetworkNode(NetworkNode *node) {
		this->node = node;
	}
	/**
	 * Returns the network node of this peer or NULL if it is not online.
	 */
	NetworkNode *getNetworkNode() {
		return node;
	}
private:
	QString name;
	PublicKey publicKey;
	NetworkNode *node;
};

#endif

