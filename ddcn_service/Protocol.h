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
		JobRequest = FirstType,
		JobRequestAccepted,
		JobRequestRejected,
		JobData,
		JobFinished,
		QueryNetworkResources,
		NetworkResourcesAvailable,
		QueryNodeStatus,
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

class Packet {
public:
	Packet() {
	}
	Packet(const Packet &other) : data(other.data) {
	}
	template<typename T> Packet(PacketType::List type, const T &payload) {
		qDebug("T: %d", sizeof(T));
		PacketData *packetData = new PacketData(type, sizeof(T));
		std::memcpy(packetData->getPayload(), &payload, sizeof(T));
		this->data = packetData;
	}
	Packet(PacketType::List type, unsigned int size = 0, void *data = NULL) {
		PacketData *packetData = new PacketData(type, size);
		if (data) {
			std::memcpy(packetData->getPayload(), data, size);
		}
		this->data = packetData;
	}

	PacketType::List getType() const {
		if (!data) {
			return PacketType::Invalid;
		} else {
			return data.constData()->getType();
		}
	}
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
	unsigned int getPayloadSize() const {
		if (!data) {
			return 0;
		}
		return data.constData()->getPayloadSize();
	}

	const void *getRawData() const {
		if (!data) {
			return NULL;
		}
		return data.constData()->getData();
	}
	unsigned int getRawSize() const {
		if (!data) {
			return 0;
		}
		return data.constData()->getSize();
	}

	Packet &operator=(const Packet &other) {
		data = other.data;
		return *this;
	}

	bool isValid() const {
		if (!data) {
			return false;
		} else {
			return true;
		}
	}

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
		return Packet((PacketType::List)header->type, payloadSize, payload);
	}
private:
	QSharedDataPointer<PacketData> data;
};

// Template specialization for easy insertion of raw byte arrays
template<> inline Packet::Packet<QByteArray>(PacketType::List type, const QByteArray &data) {
	PacketData *packetData = new PacketData(type, data.size());
	std::memcpy(packetData->getPayload(), data.data(), data.size());
	this->data = packetData;
}

#endif
