#ifndef PACKET_H_
#define PACKET_H_


typedef struct tagComPacket {
	DWORD		Reserved;
	WORD		ComID;
	WORD		ComIDExtension;
	DWORD		OutstandingData;
	DWORD		MinTransfer;
	DWORD		Length;
} TCGCOMPACKET, *LPTCGCOMPACKET;

typedef struct tagPacket {
	DWORD		TperSessionNumber;
	DWORD		HostSessionNumber;
	DWORD		SequenceNumber;
	WORD		Reserved;
	WORD		AckType;
	DWORD		Acknowledgement;
	DWORD		Length;
} TCGPACKET, *LPTCGPACKET;

typedef struct tagSubPacket {
	BYTE		Reserved[6];
	WORD		Kind;
	DWORD		Length;
} TCGSUBPACKET, *LPTCGSUBPACKET;

typedef struct tagTcgPacket {
	TCGCOMPACKET	ComPacket;
	TCGPACKET		Packet;
	TCGSUBPACKET	SubPacket;
	BYTE			Payload[1];
} TCGFULLPACKET, *LPTCGFULLPACKET;


#endif /* PACKET_H_ */
