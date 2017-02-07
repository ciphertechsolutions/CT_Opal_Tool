#include"stdafx.h"
#include"Memory.h"
#include"AtaDrive.h"


/* The task file for passing parameters to the ATA command. */
typedef struct tagTaskFile {
	WORD		Feature;		/* Feature register contents. */
	WORD		Sectors;		/* Number of sectors to transfer. */
	QWORD		Lba;			/* Starting LBA of transfer. */
	BYTE		Device;			/* Device: Master or Slave. */
	BYTE		Cmd;			/* ATA Command. */
} TASKFILE, *LPTASKFILE;


/* Get a specific byte from a larger (e.g dword) value. */
#define GETBYTE(x, BitShift)		((BYTE)(((x) >> (BitShift)) & 0xff))

/* Sector buffers must be page-aligned. */
#define SectorAlloc(Size)			VirtualAlloc(NULL, Size, MEM_COMMIT, PAGE_READWRITE)
#define SectorFree(Buffer)			VirtualFree(Buffer, 0, MEM_RELEASE)


/*
 * Initialize the SCSI pass through direct structure.
 */
static void InitScsiPassThroughDirect(PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptd, LPBYTE Cdb, BYTE CdbLength, BYTE DataFlag, LPBYTE TransferBuffer, DWORD TransferLength)
{
	sptd->sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	sptd->sptd.ScsiStatus = 0;
	sptd->sptd.PathId = 0;
	sptd->sptd.TargetId = 0;
	sptd->sptd.Lun = 0;
	sptd->sptd.SenseInfoLength = SENSE_BUFFER_LENGTH;
	sptd->sptd.TimeOutValue = 20;
	sptd->sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuffer);

	sptd->sptd.CdbLength = CdbLength;
	sptd->sptd.DataIn = DataFlag;
	sptd->sptd.DataTransferLength = TransferLength;
	sptd->sptd.DataBuffer = TransferBuffer;

	memcpy(&(sptd->sptd.Cdb), Cdb, CdbLength);
}


/*
 * Send an ATA command to the drive over a USB interface.
 */
static BOOL SendAtaCommandOverUsb(LPATADRIVE hDrive, USHORT Flags, DWORD DataLength, LPVOID Buffer, LPTASKFILE TaskFile)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER	sptd;
	DWORD									nRead;
	BOOL									Result;
	BYTE									Protocol;
	BYTE									DataFlag;
	BYTE									CdbLength;
	BYTE									Cdb[16];

	/* Set up pass through values. */
	memset(Cdb, 0, sizeof(Cdb));
	DataFlag = 0;
	if(Flags & ATA_FLAGS_DATA_IN) {
		DataFlag = SCSI_IOCTL_DATA_IN;
		Protocol = 4;
		Cdb[2] |= 8;
		Cdb[2] |= 6;
	}
	if(Flags & ATA_FLAGS_DATA_OUT) {
		DataFlag = SCSI_IOCTL_DATA_OUT;
		Protocol = 5;
		Cdb[2] |= 6;
	}
	if(Flags & ATA_FLAGS_USE_DMA) {
		Protocol += 6;
	}
	Cdb[1] = Protocol << 1;
	if(Flags & ATA_FLAGS_48BIT_COMMAND) {
		CdbLength = 16;
		Cdb[0] = 0x85;
		Cdb[1] |= 1;								/* The extend bit. */
		Cdb[3] = GETBYTE(TaskFile->Feature, 8);
		Cdb[4] = GETBYTE(TaskFile->Feature, 0);
		Cdb[5] = GETBYTE(TaskFile->Sectors, 8);
		Cdb[6] = GETBYTE(TaskFile->Sectors, 0);
		Cdb[8] = GETBYTE(TaskFile->Lba, 0);
		Cdb[10] = GETBYTE(TaskFile->Lba, 8);
		Cdb[12] = GETBYTE(TaskFile->Lba, 16);
		Cdb[7] = GETBYTE(TaskFile->Lba, 24);
		Cdb[9] = GETBYTE(TaskFile->Lba, 32);
		Cdb[11] = GETBYTE(TaskFile->Lba, 40);
		Cdb[13] = GETBYTE(TaskFile->Device, 0);
		Cdb[14] = GETBYTE(TaskFile->Cmd, 0);
	} else {
		CdbLength = 12;
		Cdb[0] = 0xA1;
		Cdb[3] = GETBYTE(TaskFile->Feature, 0);
		Cdb[4] = GETBYTE(TaskFile->Sectors, 0);
		Cdb[5] = GETBYTE(TaskFile->Lba, 0);
		Cdb[6] = GETBYTE(TaskFile->Lba, 8);
		Cdb[7] = GETBYTE(TaskFile->Lba, 16);
		Cdb[8] = GETBYTE(TaskFile->Lba, 24) & 0x0f;
		Cdb[8] |= GETBYTE(TaskFile->Device, 0) & 0xf0;
		Cdb[9] = GETBYTE(TaskFile->Cmd, 0);
	}
	InitScsiPassThroughDirect(&sptd, Cdb, CdbLength, DataFlag, (LPBYTE)Buffer, DataLength);

	/* Send the command to the drive. */
	nRead = 0;
	Result = DeviceIoControl(hDrive->hDrive, IOCTL_SCSI_PASS_THROUGH_DIRECT, &sptd, sizeof(sptd), &sptd, sizeof(sptd), &nRead, NULL);

	/* Check for command failure. */
	if(nRead < sizeof(SCSI_PASS_THROUGH_DIRECT)) {
		Result = FALSE;
	}
	if(sptd.sptd.ScsiStatus != 0) {
		Result = FALSE;
	}

	/* Return the result. */
	return Result;
}


/*
 * Send an ATA command to the drive over an ATA interface.
 */
static BOOL SendAtaCommandOverAta(LPATADRIVE hDrive, USHORT Flags, DWORD DataLength, LPVOID Buffer, LPTASKFILE TaskFile)
{
	ATA_PASS_THROUGH_DIRECT		Ptd;
	DWORD						nRead;
	BOOL						Result;

	/* Set up pass through values. */
	memset(&Ptd, 0, sizeof(ATA_PASS_THROUGH_DIRECT));
	Ptd.Length = sizeof(ATA_PASS_THROUGH_DIRECT);
	Ptd.TimeOutValue = 60;
	Ptd.AtaFlags = Flags | (USHORT)ATA_FLAGS_DRDY_REQUIRED;
	Ptd.DataTransferLength = DataLength;
	Ptd.DataBuffer = Buffer;

	/* Set up the task files. */
	Ptd.CurrentTaskFile[0] = GETBYTE(TaskFile->Feature, 0);
	Ptd.CurrentTaskFile[1] = GETBYTE(TaskFile->Sectors, 0);
	Ptd.CurrentTaskFile[2] = GETBYTE(TaskFile->Lba, 0);
	Ptd.CurrentTaskFile[3] = GETBYTE(TaskFile->Lba, 8);
	Ptd.CurrentTaskFile[4] = GETBYTE(TaskFile->Lba, 16);
	Ptd.CurrentTaskFile[6] = GETBYTE(TaskFile->Cmd, 0);
	if(Flags & ATA_FLAGS_48BIT_COMMAND) {
		Ptd.CurrentTaskFile[5] = GETBYTE(TaskFile->Device, 0);
		Ptd.PreviousTaskFile[0] = GETBYTE(TaskFile->Feature, 8);
		Ptd.PreviousTaskFile[1] = GETBYTE(TaskFile->Sectors, 8);
		Ptd.PreviousTaskFile[2] = GETBYTE(TaskFile->Lba, 24);
		Ptd.PreviousTaskFile[3] = GETBYTE(TaskFile->Lba, 32);
		Ptd.PreviousTaskFile[4] = GETBYTE(TaskFile->Lba, 40);
	} else {
		Ptd.CurrentTaskFile[5] = (GETBYTE(TaskFile->Device, 0) & 0xf0) | (GETBYTE(TaskFile->Lba, 24) & 0x0f);
	}

	/* Send the command to the drive. */
	nRead = 0;
	Result = DeviceIoControl(hDrive->hDrive, IOCTL_ATA_PASS_THROUGH_DIRECT, &Ptd, sizeof(ATA_PASS_THROUGH_DIRECT), &Ptd, sizeof(ATA_PASS_THROUGH_DIRECT), &nRead, NULL);

	/* Check for command failure. */
	if(nRead < Ptd.Length) {
		Result = FALSE;
	}
	if(Ptd.CurrentTaskFile[6] & 0x01) {
		Result = FALSE;
	}

	/* Return the result. */
	return Result;
}


/*
 * Send a command to the hard drive.
 */
static BOOL SendAtaCommand(LPATADRIVE hDrive, USHORT Flags, DWORD DataLength, LPVOID Buffer, LPTASKFILE TaskFile)
{
	if(hDrive != NULL) {
		switch(hDrive->BusType) {
			case BusTypeAta:
			case BusTypeSata:
				return SendAtaCommandOverAta(hDrive, Flags, DataLength, Buffer, TaskFile);
				break;
			case BusTypeUsb:
				return SendAtaCommandOverUsb(hDrive, Flags, DataLength, Buffer, TaskFile);
				break;
		}
	}

	return FALSE;
}


/*
 * Close the drive.
 */
void CloseAtaDrive(LPATADRIVE hDrive)
{
	if(hDrive != NULL) {
		if(hDrive->hDrive != INVALID_HANDLE_VALUE) {
			CloseHandle(hDrive->hDrive);
		}
		if(hDrive->Scratch != NULL) {
			SectorFree(hDrive->Scratch);
		}
		MemFree(hDrive);
	}
}


/*
 * Switch every two bytes in a buffer.
 */
static void SwitchBuffer(LPSTR Buffer, int Size)
{
	TCHAR	Temp;
	int		i;

	for(i=0; i<Size; i+=2) {
		Temp = Buffer[i+1];
		Buffer[i+1] = Buffer[i];
		Buffer[i] = Temp;
	}
}


/*
 * Opens a drive and initializes the structure.
 */
LPATADRIVE OpenAtaDrive(LPTSTR Drive)
{
	STORAGE_ADAPTER_DESCRIPTOR	Descriptor;
	STORAGE_PROPERTY_QUERY		Query;
	STORAGE_DEVICE_NUMBER		sdn;
	LPATADRIVE					hDrive;
	TASKFILE					TaskFile;
	DWORD						nReturned;
	BOOL						Result;
	BYTE						Sum;
	int							i;

	/* Allocate memory. */
	hDrive = (LPATADRIVE)MemCalloc(sizeof(ATADRIVE));
	if(hDrive == NULL) {
		return NULL;
	}
	hDrive->hDrive = INVALID_HANDLE_VALUE;

	/* Allocate scratch sector. */
	hDrive->Scratch = (LPBYTE)SectorAlloc(512);
	if(hDrive->Scratch == NULL) {
		CloseAtaDrive(hDrive);
		return NULL;
	}

	/* Open the drive. */
	hDrive->hDrive = CreateFile(Drive, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hDrive->hDrive == INVALID_HANDLE_VALUE) {
		CloseAtaDrive(hDrive);
		return NULL;
	}

	/* Determine the drive number. */
	memset(&sdn, 0, sizeof(sdn));
	DeviceIoControl(hDrive->hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &nReturned, NULL);
	hDrive->DriveNumber = sdn.DeviceNumber;

	/* Determine drive interface. */
	Query.PropertyId = StorageAdapterProperty;
	Query.QueryType = PropertyStandardQuery;
	Descriptor.Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);
	Result = DeviceIoControl(hDrive->hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &Query, sizeof(STORAGE_PROPERTY_QUERY), &Descriptor, Descriptor.Size, &nReturned, NULL);
	if(Result != FALSE) {
		hDrive->BusType = Descriptor.BusType;
	} else {
		hDrive->BusType = BusTypeUnknown;
	}

	/* Identify Device. */
	memset(&TaskFile, 0, sizeof(TaskFile));
	TaskFile.Cmd = 0xEC;
	TaskFile.Sectors = 1; /* Needed for SCSI ATA_PASS_THROUGH */
	Result = SendAtaCommand(hDrive, ATA_FLAGS_DATA_IN, 512, hDrive->Scratch, &TaskFile);
	if(Result == FALSE) {
		CloseAtaDrive(hDrive);
		return NULL;
	}

	/* Verify the checksum. */
	if(hDrive->Scratch[510] == 0xa5) {
		Sum = 0;
		for(i=0; i<512; i++) {
			Sum += hDrive->Scratch[i];
		}
		if(Sum != 0) {
			CloseAtaDrive(hDrive);
			return NULL;
		}
	}

	/* Does the drive support the trusted computing feature set?. */
	hDrive->IsTcg = FALSE;
	if((hDrive->Scratch[48*2 + 1] & 0xc0) == 0x40) {
		hDrive->IsTcg = hDrive->Scratch[48*2] & 0x01;
	}

	/* Get the hard drive model number. */
	memcpy((LPBYTE)(hDrive->Model), &(hDrive->Scratch[54]), 40);
	SwitchBuffer(hDrive->Model, 40);
	hDrive->Model[40] = 0;

	/* Get the hard drive serial number. */
	memcpy((LPBYTE)(hDrive->Serial), &(hDrive->Scratch[20]), 20);
	SwitchBuffer(hDrive->Serial, 20);
	hDrive->Serial[20] = 0;

	/* Get the drive size in sectors. */
	/* Check if 48-bit LBAs are enabled, and if so, read the drive size from there. */
	if(hDrive->Scratch[86*2 + 1] & 0x04) {
		hDrive->Size = *(QWORD *)&hDrive->Scratch[100*2];
	} else {
		hDrive->Size = *(DWORD *)&hDrive->Scratch[60*2];
	}

	/* Return the structure. */
	return hDrive;
}


/*
 * Issue the trusted send command.
 */
BOOL TrustedSend(LPATADRIVE hDrive, BOOL UseDMA, BYTE ProtocolID, WORD ComID, LPBYTE Packet, WORD Length)
{
	TASKFILE	TaskFile;
	USHORT		Flags;

	/* Set the taskfile appropriately. */
	memset(&TaskFile, 0, sizeof(TASKFILE));
	if(UseDMA != FALSE) {
		TaskFile.Cmd = 0x5F;
	} else {
		TaskFile.Cmd = 0x5E;
	}
	TaskFile.Feature = ProtocolID;
	TaskFile.Sectors = Length & 0xff;
	TaskFile.Lba = ((DWORD)ComID << 8) | ((Length >> 8) & 0xff);

	/* Set the flags. */
	Flags = ATA_FLAGS_DATA_OUT;
	if(UseDMA != FALSE) {
		Flags |= ATA_FLAGS_USE_DMA;
	}

	/* Send the command. */
	return SendAtaCommand(hDrive, Flags, Length*512, Packet, &TaskFile);
}


/*
 * Issue the trusted receive command.
 */
BOOL TrustedReceive(LPATADRIVE hDrive, BOOL UseDMA, BYTE ProtocolID, WORD ComID, LPBYTE Packet, WORD Length)
{
	TASKFILE	TaskFile;
	USHORT		Flags;

	/* Set the taskfile appropriately. */
	memset(&TaskFile, 0, sizeof(TASKFILE));
	if(UseDMA != FALSE) {
		TaskFile.Cmd = 0x5D;
	} else {
		TaskFile.Cmd = 0x5C;
	}
	TaskFile.Feature = ProtocolID;
	TaskFile.Sectors = Length & 0xff;
	TaskFile.Lba = ((DWORD)ComID << 8) | ((Length >> 8) & 0xff);

	/* Set the flags. */
	Flags = ATA_FLAGS_DATA_IN;
	if(UseDMA != FALSE) {
		Flags |= ATA_FLAGS_USE_DMA;
	}

	/* Send the command. */
	return SendAtaCommand(hDrive, Flags, Length*512, Packet, &TaskFile);
}


/*
 * Converts a bus type into a readable string.
 */
LPTSTR GetDriveBusString(LPATADRIVE hDrive)
{
	switch(hDrive->BusType) {
		case BusTypeScsi:
			return _T("SCSI");
			break;
		case BusTypeAtapi:
			return _T("ATAPI");
			break;
		case BusTypeAta:
			return _T("ATA");
			break;
		case BusType1394:
			return _T("1394");
			break;
		case BusTypeSsa:
			return _T("SSA");
			break;
		case BusTypeFibre:
			return _T("Fibre");
			break;
		case BusTypeUsb:
			return _T("USB");
			break;
		case BusTypeRAID:
			return _T("RAID");
			break;
//		case BusTypeiSCSI:
//			return _T("iSCSI");
//			break;
		case BusTypeSas:
			return _T("SAS");
			break;
		case BusTypeSata:
			return _T("SATA");
			break;
		case BusTypeUnknown:
		default:
			return _T("Unknown");
			break;
	}
}
