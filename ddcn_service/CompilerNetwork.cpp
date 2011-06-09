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

THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND ANY EXPRESS OR
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

#include "CompilerNetwork.h"

CompilerNetwork::CompilerNetwork() : encryptionEnabled(true) {
	localNode = new NetworkNode();
}
CompilerNetwork::~CompilerNetwork() {
	delete localNode;
}

void CompilerNetwork::setPeerName(QString peerName) {
	this->peerName = peerName;
	emit peerNameChanged(peerName);
}
QString CompilerNetwork::getPeerName() {
	return peerName;
}

void CompilerNetwork::setEncryption(bool encryptionEnabled) {
	this->encryptionEnabled = encryptionEnabled;
	emit encryptionChanged(encryptionEnabled);
}
bool CompilerNetwork::getEncryption() {
	return encryptionEnabled;
}

void CompilerNetwork::setKeys(QString publicKey, QString privateKey) {
	this->publicKey = publicKey;
	this->privateKey = privateKey;
	emit publicKeyChanged(publicKey);
	emit privateKeyChanged(privateKey);
}
void CompilerNetwork::generateKeys() {
	// TODO
}
QString CompilerNetwork::getPublicKey() {
	return publicKey;
}
QString CompilerNetwork::getPrivateKey() {
	return privateKey;
}

void CompilerNetwork::addTrustedPeer(QString name, QString publicKey) {
	// TODO
}
void CompilerNetwork::removeTrustedPeer(QString name, QString publicKey) {
	// TODO
}

void CompilerNetwork::addTrustedGroup(QString name, QString publicKey) {
	// TODO
}
void CompilerNetwork::removeTrustedGroup(QString name, QString publicKey) {
	// TODO
}

void CompilerNetwork::addGroupMembership(QString name, QString publicKey, QString privateKey) {
	// TODO
}
void CompilerNetwork::removeGroupMembership(QString name, QString publicKey) {
	// TODO
}

bool CompilerNetwork::canAcceptOutgoingJobRequest() {
	// TODO
	return false;
}
bool CompilerNetwork::delegateOutgoingJob(Job *job) {
	// TODO
	return false;
}

