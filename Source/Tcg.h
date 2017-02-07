#ifndef TCG_H_
#define TCG_H_

/* Needed include file. */
#include"Packet.h"


/* The structure for TCG drives. */
typedef struct tagTcgDrive {
	LPATADRIVE		hDrive;			/* Pointer to the ATA drive information. */
	BOOL			UseDMA;			/* True to use DMA versions of the trusted commands, false to use the PIO ones. */
	DWORD			BufferSize;		/* The size of the TPer's transmit or receive buffer, whichever is smaller. */
	WORD			FirstComID;		/* The first com ID of the appropriate feature set. */
	BOOL			IsSamsung;		/* True if we think the drive is a first generation Samsung drive. */
	LPTCGFULLPACKET	ComPacket;		/* A pointer to the ComPacket to be transferred. */
} TCGDRIVE, *LPTCGDRIVE;


/* The structure used in sessions. */
typedef struct tagSession {
	LPTCGDRIVE			hDrive;				/* Handle to the drive we're talking to. */
	DWORD				HostNumber;			/* The host's session identifier. */
	DWORD				TperNumber;			/* The TPer's session identifier. */
} TCGSESSION, *LPTCGSESSION;


/* The structure used for authorizations. */
typedef struct tagTcgAuth {
	BOOL				IsValid;			/* True if the information is valid, false for no user specified. */
	BYTE				Authority[8];		/* The authority Uid. */
	BYTE				Credentials[32];	/* Authority's password, maximum 32 bytes per Core Spec. */
	DWORD				Size;				/* The size, in bytes, of the credentials. */
} TCGAUTH, *LPTCGAUTH;


/* The list of properties to send to the TPer. */
typedef struct tagProperties {
	LPTSTR		String;
	DWORD		Value;
} PROPERTIES, *LPPROPERTIES;


/* Function prototypes. */
BOOL Level0Discovery(LPTCGDRIVE hDrive);
DWORD GetLockingFeatures(LPTCGDRIVE hDrive);
BOOL IsLockingEnabled(LPTCGDRIVE hDrive);
WORD GetTCGDriveType(LPTCGDRIVE hDrive);
LPTSTR GetDriveTypeString(WORD Type);
LPTABLE Level1Discovery(LPTCGDRIVE hDrive, LPPROPERTIES Properties);
BYTE StartSession(LPTCGSESSION Session, LPTCGDRIVE hDrive, LPBYTE Sp, LPTCGAUTH TcgAuth);
BOOL CheckAuth(LPTCGDRIVE hDrive, LPBYTE SpUid, LPTCGAUTH TcgAuth);
BYTE ReadByteTable(LPTCGSESSION Session, LPBYTE TableUid, LPBYTE Buffer, DWORD Start, DWORD Size);
BYTE WriteByteTable(LPTCGSESSION Session, LPBYTE TableUid, LPBYTE Buffer, DWORD Start, DWORD Size);
LPTABLECELL ReadTableCell(LPTCGSESSION Session, LPBYTE RowUid, int Column);
BOOL ReadTableCellBytes(LPTCGSESSION Session, LPBYTE RowUid, int Column, LPBYTE Buffer, LPDWORD Size);
BOOL ReadTableCellDword(LPTCGSESSION Session, LPBYTE RowUid, int Column, LPDWORD Value);
LPTABLE ReadTable(LPTCGSESSION Session, LPBYTE TableUid);
LPTABLE ReadTableNoSession(LPTCGDRIVE hDrive, LPBYTE Sp, LPBYTE TableUid, LPTCGAUTH TcgAuth);
BYTE GetRandom(LPTCGSESSION Session, int Size, LPBYTE Bytes);
BYTE Activate_SP(LPTCGSESSION Session, LPBYTE Sp);
BYTE Revert(LPTCGSESSION Session, LPBYTE Sp);
BYTE RevertSP(LPTCGSESSION Session, BOOL KeepGlobalKey);
BYTE ChangeAuth(LPTCGSESSION Session, LPTCGAUTH TcgAuth);
BYTE CreateRange(LPTCGSESSION Session, LPBYTE RangeUid, QWORD RangeStart, QWORD RangeLength, BOOL EnableReadLock, BOOL EnableWriteLock);
BYTE SetReadLock(LPTCGSESSION Session, LPBYTE RangeUid, BOOL ReadLock);
BYTE SetWriteLock(LPTCGSESSION Session, LPBYTE RangeUid, BOOL WriteLock);
BYTE EraseRange(LPTCGSESSION Session, LPBYTE Uid);
BYTE SetMbrState(LPTCGSESSION Session, BOOL IsEnable, BOOL IsDone);
BYTE ChangeUserState(LPTCGSESSION Session, LPBYTE User, BOOL Enable);
BOOL ReadMSID(LPTCGDRIVE hDrive, LPTCGAUTH TcgAuth);
DWORD GetByteTableSize(LPTCGDRIVE hDrive, LPBYTE Sp, LPBYTE TableUid);
BOOL EndSession(LPTCGSESSION Session);
void CloseTcgDrive(LPTCGDRIVE hDrive);
LPTCGDRIVE OpenTcgDrive(LPTSTR Drive);


#endif /* TCG_H_ */
