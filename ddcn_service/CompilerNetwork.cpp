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

CompilerNetwork::CompilerNetwork() : encryptionEnabled(true), freeLocalSlots(0),
		lastJobId(0), settings(QSettings::IniFormat, QSettings::UserScope, "ddcn", "ddcn") {
	// Load peer name and public key from configuration
	if (!settings.value("name").isValid()) {
		settings.setValue("name", "ddcn_node");
	}
	QString name = settings.value("general/name").toString();
	// Load key from file in the settings directory
	QString keyFile = QFileInfo(settings.fileName()).absolutePath() + "/privkey.pem";
	PrivateKey key = PrivateKey::load(keyFile);
	if (!key.isValid()) {
		key = PrivateKey::generate(2048);
		if (!key.save(keyFile)) {
			qWarning("Warning: Could not save local private key!");
		}
	}
	// Load trusted peers/groups etc from file
	// TODO
	// Initialize ariba
	network = new NetworkInterface(name, key);
	connect(network,
	        SIGNAL(peerConnected(NetworkNode*, QString, PublicKey)),
	        this,
	        SLOT(onPeerConnected(NetworkNode*, QString, PublicKey)));
	connect(network,
	        SIGNAL(peerDisconnected(NetworkNode*)),
	        this,
	        SLOT(onPeerDisconnected(NetworkNode*)));
	connect(network,
	        SIGNAL(peerChanged(NetworkNode*, QString)),
	        this,
	        SLOT(onPeerChanged(NetworkNode*, QString)));
	connect(network,
	        SIGNAL(messageReceived(NetworkNode*, Packet)),
	        this,
	        SLOT(onMessageReceived(NetworkNode*, Packet)));
	connect(network,
	        SIGNAL(groupMessageReceived(McpoGroup*, NetworkNode*, Packet)),
	        this,
	        SLOT(onGroupMessageReceived(McpoGroup*, NetworkNode*, Packet)));
}
CompilerNetwork::~CompilerNetwork() {
	foreach (OutgoingJobRequest *request, outgoingJobRequests) {
		delete request;
	}
	foreach (IncomingJobRequest *request, incomingJobRequests) {
		delete request;
	}
	delete network;
}

void CompilerNetwork::setPeerName(QString peerName) {
	this->peerName = peerName;
	network->setName(peerName);
	emit peerNameChanged(peerName);
}
QString CompilerNetwork::getPeerName() {
	return peerName;
}

void CompilerNetwork::setEncryption(bool encryptionEnabled) {
	this->encryptionEnabled = encryptionEnabled;
	// TODO: Disable encryption in NetworkNode
	emit encryptionChanged(encryptionEnabled);
}
bool CompilerNetwork::getEncryption() {
	return encryptionEnabled;
}

void CompilerNetwork::setLocalKey(const PrivateKey &privateKey) {
	localKey = privateKey;
	// TODO: Change key in NetworkInterface
	//network->changeIdentity(peerName, publicKey);
	// Save key
	QString keyFile = QFileInfo(settings.fileName()).absolutePath() + "/privkey.pem";
	if (!privateKey.save(keyFile)) {
		qWarning("Warning: Could not save local private key!");
	}
	emit localKeyChanged(privateKey);
}
void CompilerNetwork::generateLocalKey() {
	// TODO: Let the user choose the key length
	PrivateKey privateKey = PrivateKey::generate(2048);
	setLocalKey(privateKey);
}
PrivateKey CompilerNetwork::getLocalKey() {
	return localKey;
}

void CompilerNetwork::addTrustedPeer(QString name, const PublicKey &publicKey) {
	TrustedPeer *existing = getTrustedPeer(publicKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	TrustedPeer *trustedPeer = new TrustedPeer(name, publicKey);
	trustedPeers.append(trustedPeer);
	NetworkNode *node = network->getNetworkNode(publicKey);
	if (node) {
		node->setTrustedPeer(trustedPeer);
		trustedPeer->setNetworkNode(node);
	}
	saveSettings();
	emit trustedPeersChanged(trustedPeers);
}
void CompilerNetwork::removeTrustedPeer(QString name, const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		if (trustedPeer->getPublicKey() == publicKey) {
			if (trustedPeer->getNetworkNode() != NULL) {
				trustedPeer->getNetworkNode()->setTrustedPeer(NULL);
			}
			trustedPeers.removeOne(trustedPeer);
			delete trustedPeer;
			return;
		}
	}
	saveSettings();
	emit trustedPeersChanged(trustedPeers);
}

void CompilerNetwork::addTrustedGroup(QString name, const PublicKey &publicKey) {
	TrustedGroup *existing = getTrustedGroup(publicKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	TrustedGroup *trustedGroup = new TrustedGroup(name, publicKey);
	trustedGroups.append(trustedGroup);
	// Join the group
	ariba::ServiceID serviceId = McpoGroup::getServiceIdFromPublicKey(publicKey);
	trustedGroup->setMcpoGroup(network->joinGroup(serviceId));
	saveSettings();
	emit trustedGroupsChanged(trustedGroups);
}
void CompilerNetwork::removeTrustedGroup(QString name, const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		if (trustedGroup->getPublicKey() == publicKey) {
			network->leaveGroup(trustedGroup->getMcpoGroup());
			trustedGroups.removeOne(trustedGroup);
			delete trustedGroup;
			return;
		}
	}
	saveSettings();
	emit trustedGroupsChanged(trustedGroups);
}

void CompilerNetwork::addGroupMembership(QString name, const PrivateKey &privateKey) {
	GroupMembership *existing = getGroupMembership(privateKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	GroupMembership *groupMembership = new GroupMembership(name, privateKey);
	groupMemberships.append(groupMembership);
	// Join the group
	ariba::ServiceID serviceId = McpoGroup::getServiceIdFromPublicKey(privateKey);
	groupMembership->setMcpoGroup(network->joinGroup(serviceId));
	saveSettings();
	emit groupMembershipsChanged(groupMemberships);
}
void CompilerNetwork::removeGroupMembership(QString name, const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (GroupMembership *groupMembership, groupMemberships) {
		if (groupMembership->getPrivateKey() == publicKey) {
			network->leaveGroup(groupMembership->getMcpoGroup());
			groupMemberships.removeOne(groupMembership);
			delete groupMembership;
			return;
		}
	}
	saveSettings();
	emit groupMembershipsChanged(groupMemberships);
}

void CompilerNetwork::delegateOutgoingJob(Job *job) {
	addWaitingJob(job);
	if (freeRemoteSlots.size() > 0) {
		// Create a job request for the job
		createJobRequests();
		// If the free remote slots are too little, ask all nodes for free slots
		// TODO: Better threshold here, like 0.5 * maxFreeRemoteSlots
		if (freeRemoteSlots.count() < 4) {
			askForFreeSlots();
		}
	} else {
		askForFreeSlots();
	}
}
Job *CompilerNetwork::cancelOutgoingJob() {
	// Cancel a job which has not yet been delegated
	Job *job = removeWaitingJob();
	if (job) {
		return job;
	}
	// TODO: Check whether we can abort an already delegated job
	return NULL;
}
void CompilerNetwork::rejectIncomingJob(Job *job) {
	// TODO
}

void CompilerNetwork::setFreeLocalSlots(unsigned int localSlots) {
	freeLocalSlots = localSlots;
}
unsigned int CompilerNetwork::getFreeLocalSlots() {
	return freeLocalSlots;
}

void CompilerNetwork::queryNetworkStatus() {
	qDebug("queryNetworkStatus");
	// Send network status requests to all connected peers
	Packet packet(PacketType::QueryNodeStatus);
	network->sendToAll(packet);
}

void CompilerNetwork::onPeerConnected(NetworkNode *node, QString name,
		const PublicKey &publicKey) {
	TrustedPeer *trustedPeer = getTrustedPeer(publicKey);
	if (trustedPeer) {
		trustedPeer->setNetworkNode(node);
	}
	// TODO: Emit spareResources() ?
}
void CompilerNetwork::onPeerDisconnected(NetworkNode *node) {
	if (node->getTrustedPeer()) {
		node->getTrustedPeer()->setNetworkNode(NULL);
	}
	// Reject local jobs delegated to this node
	for (int i = delegatedJobs.size() - 1; i >= 0; i--) {
		if (delegatedJobs[i]->getTargetPeer() == node) {
			emit finishedJob(delegatedJobs[i]->getJob(), false, false);
			delegatedJobs.removeAt(i);
		}
	}
	// Abort remote jobs from this node
	for (int i = incomingJobs.size() - 1; i >= 0; i--) {
		if (incomingJobs[i]->getTargetPeer() == node) {
			// Kill the job
			// TODO: CompilerService does not yet react to this
			emit remoteJobAborted(incomingJobs[i]->getJob());
			// Delete the job
			delete incomingJobs[i]->getJob();
			delete incomingJobs[i];
			incomingJobs.removeAt(i);
		}
	}
	for (int i = incomingJobRequests.size() - 1; i >= 0; i--) {
		if (incomingJobRequests[i]->source == node) {
			incomingJobRequests.removeAt(i);
		}
	}
	// Abort job requests directed to this node
	bool requestsRemoved = false;
	for (int i = outgoingJobRequests.size() - 1; i >= 0; i--) {
		if (outgoingJobRequests[i]->target == node) {
			outgoingJobRequests.removeAt(i);
			requestsRemoved = true;
		}
	}
	if (requestsRemoved) {
		// Create new requests if necessary
		// TODO: Also look for network resources if necessary
		createJobRequests();
	}
	QList<IncomingJobRequest*> incomingJobRequests;
}
void CompilerNetwork::onPeerChanged(NetworkNode *node, QString name) {
	// TODO: Nothing to do here? Maybe update TrustedPeer name
}
void CompilerNetwork::onMessageReceived(NetworkNode *node, const Packet &packet) {
	switch (packet.getType()) {
		case PacketType::JobRequest:
			onIncomingJobRequest(node, packet);
			break;
		case PacketType::JobRequestAccepted:
			onJobRequestAccepted(node, packet);
			break;
			break;
		case PacketType::JobRequestRejected:
			onJobRequestRejected(node, packet);
			break;
			break;
		case PacketType::JobData:
			onJobData(node, packet);
			break;
		case PacketType::JobDataReceived:
			onJobDataReceived(node, packet);
			break;
		case PacketType::JobFinished:
			onJobFinished(node, packet);
			break;
		case PacketType::AbortJob:
			onAbortJob(node, packet);
			break;
		case PacketType::QueryNetworkResources:
			qDebug("Node asking for available resources.");
			reportNetworkResources(node);
			break;
		case PacketType::NetworkResourcesAvailable:
			onNetworkResourcesAvailable(node, packet);
			break;
		case PacketType::QueryNodeStatus:
			qDebug("Node asking for node status.");
			reportNodeStatus(node);
			break;
		case PacketType::NodeStatus:
			onNodeStatusChanged(node, packet);
			break;
		default:
			qWarning("Warning: Unknown package type received.");
			break;
	}
}
void CompilerNetwork::onGroupMessageReceived(McpoGroup *group, NetworkNode *node,
		const Packet &packet) {
	switch (packet.getType()) {
		case PacketType::QueryNetworkResources:
			qDebug("Node asking for network resources.");
			reportNetworkResources(node);
			break;
		default:
			qWarning("Warning: Unknown group package type received.");
			break;
	}
}

void CompilerNetwork::onPreprocessingFinished(Job *job, int returnValue,
		const QByteArray &stdout, const QByteArray &stderr) {
	int waitingJobIndex = -1;
	for (int i = 0; i < waitingPreprocessingJobs.count(); i++) {
		if (waitingPreprocessingJobs[i] == job) {
			waitingJobIndex = i;
			break;
		}
	}
	if (waitingJobIndex == -1) {
		// Nothing to do here, the job was cancelled by CompilerService
		return;
	}
	// If an error occurred, the job is finished
	if (returnValue != 0) {
		job->setFinished(returnValue, stdout, stderr);
		return;
	}
	// Insert job into waitingPreprocessedJobs if it is in waitingPreprocessingJobs
	// TODO: We also have to store preprocessor output!
	waitingPreprocessingJobs.removeAt(waitingJobIndex);
	waitingPreprocessedJobs.append(job);
	// Delegate the job if possible
	// TODO: We have to keep a list of the requests which have been accepted but
	// no job has been sent
}

TrustedPeer *CompilerNetwork::getTrustedPeer(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		if (trustedPeer->getPublicKey() == publicKey) {
			return trustedPeer;
		}
	}
	return NULL;
}
TrustedGroup *CompilerNetwork::getTrustedGroup(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		if (trustedGroup->getPublicKey() == publicKey) {
			return trustedGroup;
		}
	}
	return NULL;
}
GroupMembership *CompilerNetwork::getGroupMembership(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (GroupMembership *groupMembership, groupMemberships) {
		if (groupMembership->getPrivateKey() == publicKey) {
			return groupMembership;
		}
	}
	return NULL;
}

void CompilerNetwork::saveSettings() {
	// TODO: Save trusted groups, trusted peers, group memberships
}

void CompilerNetwork::askForFreeSlots() {
	// Send a message to all trusted peers
	// TODO: Maybe include the toolchain version here? Might reduce overall
	// traffic generated
	Packet packet(PacketType::QueryNetworkResources);
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		network->send(trustedPeer->getNetworkNode(), packet);
	}
	// The packet sent to the groups has to look different as we have to
	// include the group key
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		// Create a new packet for each group containing the group key
		QByteArray payload = trustedGroup->getPublicKey().toDER();
		packet = Packet(PacketType::QueryNetworkResources, payload);
		network->send(trustedGroup->getMcpoGroup(), packet);
	}
}

void CompilerNetwork::reportNodeStatus(NetworkNode *node) {
	NodeStatusPacket nodeStatus;
	// TODO: Get real data
	nodeStatus.maxThreads = 2;
	nodeStatus.currentThreads = 1;
	nodeStatus.delegatedJobs = 0;
	nodeStatus.remoteJobs = 0;
	nodeStatus.groupCount = groupMemberships.count();
	QByteArray packetData;
	packetData.resize(sizeof(nodeStatus));
	memcpy(packetData.data(), &nodeStatus, sizeof(nodeStatus));
	QDataStream stream(&packetData, QIODevice::Append);
	foreach (GroupMembership *groupMembership, groupMemberships) {
		QByteArray key = groupMembership->getPrivateKey().toDER();
		stream << (unsigned int)key.size();
		stream << key;
	}
	Packet packet(PacketType::NodeStatus, packetData);
	network->send(node, packet);
}
void CompilerNetwork::reportNetworkResources(NetworkNode *node) {
	Packet packet(PacketType::NetworkResourcesAvailable, freeLocalSlots);
	network->send(node, packet);
}

void CompilerNetwork::onNodeStatusChanged(NetworkNode *node, const Packet &packet) {
	qDebug("onNodeStatusChanged");
	// Parse node info
	const NodeStatusPacket *nodeStatus = packet.getPayload<NodeStatusPacket>();
	if (!nodeStatus) {
		qWarning("Warning: Received too small packet (%d instead of %d).",
				packet.getPayloadSize(), (int)sizeof(*nodeStatus));
		return;
	}
	NodeStatus status;
	status.currentThreads = nodeStatus->currentThreads;
	status.maxThreads =nodeStatus->maxThreads;
	status.delegatedJobs = nodeStatus->delegatedJobs;
	status.localJobs = nodeStatus->localJobs;
	status.remoteJobs = nodeStatus->remoteJobs;
	// Parse group info
	unsigned int groupCount = nodeStatus->groupCount;
	char *groupData = (char*)packet.getPayloadData() + sizeof(NodeStatusPacket);
	QByteArray remaining(groupData, packet.getPayloadSize() - sizeof(NodeStatusPacket));
	QDataStream stream(remaining);
	QStringList groupKeys;
	for (unsigned int i = 0; i < groupCount; i++) {
		unsigned int keyLength = 0;
		stream >> keyLength;
		QByteArray keyData;
		stream >> keyData;
		if (stream.atEnd()) {
			break;
		}
		PublicKey key = PublicKey::fromDER(keyData);
		groupKeys.append(key.toPEM());
	}
	QString fingerprint = node->getPublicKey().fingerprint();
	emit nodeStatusChanged(node->getPublicKey().toPEM(), fingerprint, status, groupKeys);
}

void CompilerNetwork::onNetworkResourcesAvailable(NetworkNode *node, const Packet &packet) {
	qDebug("onNetworkResourcesAvailable");
	if (packet.getPayloadSize() != sizeof(unsigned int)) {
		qWarning("Wrong packet size (onNetworkResourcesAvailable(): %d instead of %d).",
				packet.getPayloadSize(), (int)sizeof(unsigned int));
		return;
	}
	unsigned int availableCount = qFromBigEndian(*(unsigned int*)packet.getPayloadData());
	if (availableCount == 0) {
		return;
	}
	// TODO: It is possible to DOS the client here by passing in a huge value
	// This is not a proper fix as the peer can just send the packet repeatedly
	if (availableCount > 16) {
		availableCount = 16;
	}
	// Register free remote slots for later use
	FreeCompilerSlots freeSlots;
	freeSlots.node = node;
	freeSlots.slotCount = availableCount;
	freeRemoteSlots.append(freeSlots);
	// If we have waiting jobs which can now be sent out, create a request
	createJobRequests();
}

void CompilerNetwork::createJobRequests() {
	// Send job requests for waiting jobs as long as there are free slots available
	while (freeRemoteSlots.size() > 0 && outgoingJobRequests.size() < (int)getWaitingJobCount()) {
		FreeCompilerSlots &nextSlots = freeRemoteSlots[0];
		// Create request
		// TODO: The request needs a timeout so that we do not wait forever
		OutgoingJobRequest *request = new OutgoingJobRequest;
		request->target = nextSlots.node;
		request->id = generateJobId();
		outgoingJobRequests.append(request);
		Packet packet(PacketType::JobRequest, qToBigEndian(request->id));
		network->send(request->target, packet);
		// If there are not enough preprocessed waiting jobs, start preprocessing
		// for one of the waiting jobs
		int preprocessed = getPreprocessedWaitingJobCount() + getPreprocessingWaitingJobCount();
		if (outgoingJobRequests.size() > preprocessed) {
			preprocessWaitingJob();
		}
		// Remove one free slot from the list
		nextSlots.slotCount--;
		if (nextSlots.slotCount == 0) {
			freeRemoteSlots.removeFirst();
		}
	}
}

void CompilerNetwork::onIncomingJobRequest(NetworkNode *node, const Packet &packet) {
	qDebug("onIncomingJobRequest");
	// Get job id
	bool parsingError = false;
	unsigned int id = 0;
	const unsigned int *idPtr = packet.getPayload<unsigned int>();
	if (idPtr) {
		id = qFromBigEndian(*idPtr);
	} else {
		parsingError = true;
	}
	// Reject the request if necessary
	if (freeLocalSlots == 0 || parsingError) {
		// Request request
		Packet reply(PacketType::JobRequestRejected);
		network->send(node, reply);
		return;
	}
	// Create an incoming job request
	// TODO: Create a timeout so that we do not wait forever
	IncomingJobRequest *request = new IncomingJobRequest;
	request->source = node;
	request->id = id;
	incomingJobRequests.append(request);
	// Send a positive reply
	Packet reply(PacketType::JobRequestAccepted, qToBigEndian(id));
	network->send(node, reply);
	// Consume one local slot
	// TODO: Should this already be done here, or only later when the Job is created?
}

void CompilerNetwork::onJobRequestAccepted(NetworkNode *node, const Packet &packet) {
	qDebug("onJobRequestAccepted");
	// Get job id
	unsigned int id = 0;
	const unsigned int *idPtr = packet.getPayload<unsigned int>();
	if (idPtr) {
		id = qFromBigEndian(*idPtr);
	} else {
		qWarning("onJobRequestAccepted: Invalid packet received.");
		return;
	}
	// We have to check whether we have sent a job request to this peer
	// to ensure that we do not accept offers from untrusted peers
	// Also, remove the request from the list
	bool requestFound = false;
	for (int i = 0; i < outgoingJobRequests.size(); i++) {
		OutgoingJobRequest *request = outgoingJobRequests[i];
		if (request->target == node && request->id == id) {
			requestFound = true;
			// Really delegate the first job in the queue now
			Job *job = removePreprocessedWaitingJob();
			if (!job) {
				// No job is ready, so wait until a job has been preprocessed
				// TODO
			}
			delegateJob(job, request);
			// Remove the request from the list
			delete request;
			outgoingJobRequests.removeAt(i);
			break;
		}
	}
	if (!requestFound) {
		qWarning("onJobRequestAccepted: Received unknown job id.");
	}
}
void CompilerNetwork::onJobRequestRejected(NetworkNode *node, const Packet &packet) {
	qDebug("onJobRequestRejected");
	// Get job id
	unsigned int id = 0;
	const unsigned int *idPtr = packet.getPayload<unsigned int>();
	if (idPtr) {
		id = qFromBigEndian(*idPtr);
	} else {
		qWarning("onJobRequestRejected: Invalid packet received.");
		return;
	}
	// Remove outgoing job request
	bool requestFound = false;
	for (int i = 0; i < outgoingJobRequests.size(); i++) {
		OutgoingJobRequest *request = outgoingJobRequests[i];
		if (request->target == node && request->id == id) {
			requestFound = true;
			delete request;
			outgoingJobRequests.removeAt(i);
			break;
		}
	}
	if (!requestFound) {
		qWarning("onJobRequestRejected: Received unknown job id.");
	}
	// TODO: Remove all free slots from this node as it probably will not accept any later job now
	// Send new requests to other peers if necessary
	createJobRequests();
}

void CompilerNetwork::onJobData(NetworkNode *node, const Packet &packet) {
	qDebug("onJobData");
	// Create input files
	// TODO
	// Create job
	// TODO
	// Start job
	// TODO
}
void CompilerNetwork::onJobDataReceived(NetworkNode *node, const Packet &packet) {
	qDebug("onJobDataReceived");
	// Restart outgoing job timeout (we can wait longer for actual compilation
	// than for receiving the job data)
	// TODO
}
void CompilerNetwork::onJobFinished(NetworkNode *node, const Packet &packet) {
	qDebug("onJobFinished");
	// Get output data
	// TODO
	// Create output files
	// TODO
	// Finish job
	// TODO
}
void CompilerNetwork::onAbortJob(NetworkNode *node, const Packet &packet) {
	qDebug("onAbortJob");
	// Get job id
	unsigned int id = 0;
	const unsigned int *idPtr = packet.getPayload<unsigned int>();
	if (idPtr) {
		id = qFromBigEndian(*idPtr);
	} else {
		qWarning("onAbortJob: Invalid packet received.");
		return;
	}
	// Check if the job has potentially already been started
	for (int i = 0; i < incomingJobs.count(); i++) {
		if (incomingJobs[i]->getTargetPeer() == node && incomingJobs[i]->getId() == id) {
			// Kill the job
			// TODO: CompilerService does not yet react to this
			emit remoteJobAborted(incomingJobs[i]->getJob());
			// Delete the job
			delete incomingJobs[i]->getJob();
			delete incomingJobs[i];
			incomingJobs.removeAt(i);
			return;
		}
	}
	// Cancel an incoming job request if it has not
	for (int i = incomingJobRequests.size() - 1; i >= 0; i--) {
		if (incomingJobRequests[i]->source == node && incomingJobRequests[i]->id == id) {
			incomingJobRequests.removeAt(i);
			return;
		}
	}
	qWarning("onAbortJob(): Invalid job id.");
}

void CompilerNetwork::addWaitingJob(Job *job) {
	if (job->wasPreprocessed()) {
		waitingPreprocessedJobs.append(job);
	} else if (job->isPreprocessing()) {
		// We do not need to connect to the preprocessingFinished() signal as
		// we are already connected to it, CompilerNetwork started preprocessing
		// anyways
		waitingPreprocessingJobs.append(job);
	} else {
		waitingJobs.append(job);
	}
}
Job *CompilerNetwork::removeWaitingJob() {
	// Prefer to pick a job from one of the lists where little work has already
	// been done
	if (!waitingJobs.empty()) {
		Job *job = waitingJobs.last();
		waitingJobs.removeLast();
		return job;
	} else if (!waitingPreprocessingJobs.empty()) {
		Job *job = waitingPreprocessingJobs.last();
		waitingPreprocessingJobs.removeLast();
		return job;
	} else if (!waitingPreprocessedJobs.empty()) {
		Job *job = waitingPreprocessedJobs.last();
		waitingPreprocessedJobs.removeLast();
		return job;
	} else {
		return NULL;
	}
}
Job *CompilerNetwork::removePreprocessedWaitingJob() {
	if (waitingPreprocessedJobs.empty()) {
		return NULL;
	}
	Job *job = waitingPreprocessedJobs.last();
	waitingPreprocessedJobs.removeLast();
	return job;
}
unsigned int CompilerNetwork::getWaitingJobCount() {
	return waitingJobs.count() + waitingPreprocessedJobs.count()
			+ waitingPreprocessingJobs.count();
}
unsigned int CompilerNetwork::getPreprocessingWaitingJobCount() {
	return waitingPreprocessedJobs.size();
}
unsigned int CompilerNetwork::getPreprocessedWaitingJobCount() {
	return waitingPreprocessedJobs.size();
}

void CompilerNetwork::preprocessWaitingJob() {
	if (waitingJobs.empty()) {
		// This should not happen, should be checked in createJobRequests()
		qCritical("preprocessWaitingJob() called without unpreprocessed waiting jobs!");
		return;
	}
	// TODO: Should we rather pick the first job?
	Job *job = waitingJobs.last();
	waitingJobs.removeLast();
	waitingPreprocessingJobs.append(job);
	// Start preprocessing
	connect(job, SIGNAL(preProcessFinished(Job*, int, QByteArray, QByteArray)),
			this, SLOT(onPreprocessingFinished(Job*, int, QByteArray, QByteArray)));
	job->preProcess();
}

void CompilerNetwork::delegateJob(Job *job, OutgoingJobRequest *request) {
	// Collect input data
	// TODO
	// Get parameters
	// TODO
	// Create job data packet
	// TODO
}
