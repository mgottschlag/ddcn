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

#ifndef PROTOCOL_H_INCLUDED
#define PROTOCOL_H_INCLUDED

#include <QSharedData>
#include <cstdlib>
#include <cstring>
#include <QByteArray>
#include <QtEndian>
#include <stdint.h>

struct PacketType {
	enum List {
		Invalid,
		FirstType,
		/**
		 * Sent by a peer whenever it wants to delegate a job to the target.
		 *
		 * Only sent if the peer has received NetworkResourcesAvailable from the
		 * target earlier. Contains a request id which is used to later identify
		 * the job connected to this request.
		 *
		 * @todo Should also contain the toolchain version?
		 */
		JobRequest = FirstType,
		/**
		 * Sent by a peer as a response to JobRequest if the target may send the
		 * job. Contains the request id.
		 */
		JobRequestAccepted,
		/**
		 * Sent by a peer as a response to JobRequest if the target may not send
		 * the job. Contains the request id.
		 */
		JobRequestRejected,
		/**
		 * Sent by a peer after it has received JobRequestAccepted. Contains all
		 * input file data, all parameters, the toolchain version and the
		 * request id.
		 */
		JobData,
		/**
		 * Sent after the peer has successfully received JobData and begins
		 * compiling. Contains the request id.
		 */
		JobDataReceived,
		/**
		 * Sent after the peer has finished compiling. Contains all output
		 * files, all console output, the process return value and the request
		 * id. If the job was not executed at all, this contains a flag saying
		 * so.
		 */
		JobFinished,
		/**
		 * Sent by a peer which has called either JobRequest or JobData earlier.
		 *
		 * Contains a request id and cancels the request or job connected to the
		 * id.
		 */
		AbortJob,
		/**
		 * Sent to trusted peers or trusted groups whenever a peer wants to
		 * delegate any jobs, but does not have information about available
		 * resources.
		 */
		QueryNetworkResources,
		/**
		 * Sent after QueryNetworkResources has been received and if the peer
		 * has spare resources which the other peer is allowed to use.
		 */
		NetworkResourcesAvailable,
		/**
		 * Sent to all peers in the network whenever ddcn_control wants an
		 * overview about the network structure and information about all peers.
		 */
		QueryNodeStatus,
		/**
		 * Response to NetworkResourcesAvailable, contains the peer name and
		 * public key, group membership information and information about the
		 * current work load.
		 */
		NodeStatus,
		LastType = NodeStatus
	};
};

struct PacketHeader {
	// In network byte order!
	uint32_t size;
	uint8_t type;
	static void insertPacketHeaderLength(QByteArray &packet);
} __attribute__((packed));

struct NodeStatusPacket {
	unsigned short maxThreads;
	unsigned short currentThreads;
	unsigned short localJobs;
	unsigned short delegatedJobs;
	unsigned short remoteJobs;
	unsigned short groupCount;
} __attribute__((packed));

class PacketData : public QSharedData {
public:
	PacketData(const PacketData &other) : QSharedData(other) {
		size = other.size;
		data = std::malloc(size + sizeof(PacketHeader));
		memcpy(data, other.data, size + sizeof(PacketHeader));
	}
	PacketData(PacketType::List type, unsigned int size) {
		this->size = size;
		data = std::malloc(size + sizeof(PacketHeader));
		header = (PacketHeader*)data;
		header->type = type;
		header->size = qToBigEndian(size);
	}

	void *getData() {
		return data;
	}
	const void *getData() const {
		return data;
	}
	unsigned int getSize() const {
		return size + sizeof(PacketHeader);
	}
	void *getPayload() {
		return (char*)data + sizeof(PacketHeader);
	}
	const void *getPayload() const {
		return (char*)data + sizeof(PacketHeader);
	}
	unsigned int getPayloadSize() const {
		return size;
	}
	PacketType::List getType() const {
		return (PacketType::List)header->type;
	}
private:
	void *data;
	unsigned int size;

	PacketHeader *header;
};

/**
 * Class which contains a packet which can be sent over the network. This class
 * utilizes implicit data sharing via reference counting, so it can be passed
 * by value to other functions without any speed penalty.
 */
class Packet {
public:
	/**
	 * Creates an invalid packet.
	 *
	 * @see isValid()
	 */
	Packet() {
	}
	/**
	 * Copy constructor.
	 *
	 * Quite efficient because of the implicit data sharing which is happening
	 * here.
	 *
	 * @param other Packet to be copied.
	 */
	Packet(const Packet &other) : data(other.data) {
	}
	/**
	 * Creates a packet from a single value.
	 *
	 * @param type Type of the packet.
	 * @param payload Value which is inserted into the packet.
	 *
	 * @warning The payload does not need to be passed as a pointer because
	 * then the pointer would be inserted into the packet!
	 */
	template<typename T> Packet(PacketType::List type, const T &payload) {
		PacketData *packetData = new PacketData(type, sizeof(T));
		std::memcpy(packetData->getPayload(), &payload, sizeof(T));
		this->data = packetData;
	}
	/**
	 * Creates an empty packet with a certain type
	 *
	 * @param type Type of the packet.
	 */
	Packet(PacketType::List type) {
		PacketData *packetData = new PacketData(type, 0);
		data = packetData;
	}
	/**
	 * Creates a packet from raw payload data.
	 *
	 * @param type Type of the packet.
	 * @param size Size of the payload.
	 * @param data Data to be copied into the packet. Can be NULL in which case
	 * the packet content is left uninitialized and can be filled later via
	 * getPayloadData().
	 *
	 * @note Not passing a data pointer but rather filling the packet later
	 * might be faster because one copy less has to be made in may cases.
	 */
	static Packet fromData(PacketType::List type, unsigned int size = 0, void *data = NULL) {
		Packet packet;
		PacketData *packetData = new PacketData(type, size);
		if (data) {
			std::memcpy(packetData->getPayload(), data, size);
		}
		packet.data = packetData;
		return packet;
	}

	/**
	 * Returns the packet header type.
	 *
	 * @return Packet type.
	 */
	PacketType::List getType() const {
		if (!data) {
			return PacketType::Invalid;
		} else {
			return data.constData()->getType();
		}
	}
	/**
	 * Returns the payload of the packet as a single type.
	 *
	 * @return Packet payload or NULL if the packet is too small to contain the
	 * type.
	 *
	 * @note This can also be used to not read the complete payload but rather
	 * only the first part of it, the rest then can be read via
	 * getPayloadData().
	 */
	template<typename T> T *getPayload() {
		if (!data) {
			return NULL;
		}
		// Do not read more from the packet than is available
		if (getPayloadSize() < sizeof(T)) {
			return NULL;
		}
		return (T*)getPayloadData();
	}
	template<typename T> const T *getPayload() const {
		if (!data) {
			return NULL;
		}
		// Do not read more from the packet than is available
		if (getPayloadSize() < sizeof(T)) {
			return NULL;
		}
		return (const T*)getPayloadData();
	}

	/**
	 * Returns the raw payload data.
	 *
	 * @return Payload data.
	 */
	void *getPayloadData() {
		if (!data) {
			return NULL;
		}
		return data.data()->getPayload();
	}
	const void *getPayloadData() const {
		if (!data) {
			return NULL;
		}
		return data.constData()->getPayload();
	}
	/**
	 * Returns the size of the payload in bytes.
	 */
	unsigned int getPayloadSize() const {
		if (!data) {
			return 0;
		}
		return data.constData()->getPayloadSize();
	}

	/**
	 * Returns the raw data of the packet, including the packet header.
	 *
	 * @return Raw packet data.
	 */
	const void *getRawData() const {
		if (!data) {
			return NULL;
		}
		return data.constData()->getData();
	}
	/**
	 * Returns the raw packet data size.
	 *
	 * @return Raw packet data size.
	 */
	unsigned int getRawSize() const {
		if (!data) {
			return 0;
		}
		return data.constData()->getSize();
	}

	/**
	 * Copies a packet to this variable.
	 *
	 * Quite efficient because of the implicit data sharing which is happening
	 * here.
	 *
	 * @param other Packet to be copied.
	 */
	Packet &operator=(const Packet &other) {
		data = other.data;
		return *this;
	}

	/**
	 * Returns true if the packet is valid.
	 *
	 * A packet is invalid if it only was initialized with the default
	 * constructor and no valid packet was assigned to it.
	 */
	bool isValid() const {
		if (!data) {
			return false;
		} else {
			return true;
		}
	}

	/**
	 * Creates a packet from raw data received from the network.
	 *
	 * @return Resulting packet or an invalid packet if the data is corrupt.
	 */
	static Packet fromRawData(const QByteArray &data) {
		if ((size_t)data.size() < sizeof(PacketHeader)) {
			return Packet();
		}
		PacketHeader *header = (PacketHeader*)data.data();
		// Check that the packet type is valid before conversion
		if (header->type < PacketType::FirstType || header->type > PacketType::LastType) {
			return Packet();
		}
		unsigned int payloadSize = qFromBigEndian(header->size);
		// Ensure that the packet is large enough
		if ((size_t)data.size() < payloadSize + sizeof(PacketHeader)) {
			return Packet();
		}
		void *payload = (char*)data.data() + sizeof(PacketHeader);
		return fromData((PacketType::List)header->type, payloadSize, payload);
	}
private:
	QSharedDataPointer<PacketData> data;
};

/**
 * Template specialization for easy insertion of raw byte arrays
 */
template<> inline Packet::Packet<QByteArray>(PacketType::List type, const QByteArray &data) {
	PacketData *packetData = new PacketData(type, data.size());
	std::memcpy(packetData->getPayload(), data.data(), data.size());
	this->data = packetData;
}

#endif
