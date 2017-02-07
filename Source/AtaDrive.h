#ifndef ATADRIVE_H_
#define ATADRIVE_H_


/* The structure for holding information about a drive. */
typedef struct tagAtaDrive {
	HANDLE		hDrive;			/* Handle to the drive */
	DWORD		DriveNumber;	/* Number of the drive. */
	UCHAR		BusType;		/* The type of bus (e.g. ATA, SCSI, USB) */
	TCHAR		Model[41];		/* Drive model number. */
	TCHAR		Serial[21];		/* Drive serial number. */
	QWORD		Size;			/* The size of the drive in sectors. */
	BOOL		IsTcg;			/* True if the trusted computing feature set is supported. */
	LPBYTE		Scratch;		/* Scratch sector */
} ATADRIVE, *LPATADRIVE;


/* Function prototypes. */
void CloseAtaDrive(LPATADRIVE hDrive);
LPATADRIVE OpenAtaDrive(LPTSTR Drive);
BOOL TrustedSend(LPATADRIVE hDrive, BOOL UseDMA, BYTE ProtocolID, WORD ComID, LPBYTE Packet, WORD Length);
BOOL TrustedReceive(LPATADRIVE hDrive, BOOL UseDMA, BYTE ProtocolID, WORD ComID, LPBYTE Packet, WORD Length);
LPTSTR GetDriveBusString(LPATADRIVE hDrive);


#endif /* ATADRIVE_H_ */
