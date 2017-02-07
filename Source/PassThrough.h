#ifndef PASSTHROUGH_H_
#define PASSTHROUGH_H_

/* If the following file exists, include it. */
#include<ntddscsi.h>


#include<winioctl.h>
#ifndef _NTDDSCSIH_

/* The IOCTL functions that we use. */
#define IOCTL_SCSI_BASE                 FILE_DEVICE_CONTROLLER
#define IOCTL_ATA_PASS_THROUGH_DIRECT   CTL_CODE(IOCTL_SCSI_BASE, 0x040c, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

/* Scsi data directions. */
#define SCSI_IOCTL_DATA_OUT          0
#define SCSI_IOCTL_DATA_IN           1
#define SCSI_IOCTL_DATA_UNSPECIFIED  2

/* The ATA pass through direct structure. */
typedef struct _ATA_PASS_THROUGH_DIRECT {
    USHORT Length;
    USHORT AtaFlags;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR ReservedAsUchar;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    ULONG ReservedAsUlong;
    PVOID DataBuffer;
    UCHAR PreviousTaskFile[8];
    UCHAR CurrentTaskFile[8];
} ATA_PASS_THROUGH_DIRECT, *PATA_PASS_THROUGH_DIRECT;

/* ATA Pass Through Flags. */
#define ATA_FLAGS_DRDY_REQUIRED         (1 << 0)
#define ATA_FLAGS_DATA_IN               (1 << 1)
#define ATA_FLAGS_DATA_OUT              (1 << 2)
#define ATA_FLAGS_48BIT_COMMAND         (1 << 3)
#define ATA_FLAGS_USE_DMA               (1 << 4)

/* Structures and defines for the SCSI pass through command. */
typedef struct _SCSI_PASS_THROUGH_DIRECT {
    USHORT Length;
    UCHAR ScsiStatus;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR CdbLength;
    UCHAR SenseInfoLength;
    UCHAR DataIn;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    PVOID DataBuffer;
    ULONG SenseInfoOffset;
    UCHAR Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

#endif /* _NTDDSCSIH_ */


/* This is not defined in the DDK. */
#define SENSE_BUFFER_LENGTH		32
typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	SCSI_PASS_THROUGH_DIRECT		sptd;
	ULONG							Filler;
	UCHAR							ucSenseBuffer[SENSE_BUFFER_LENGTH];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;


#endif /* PASSTHROUGH_H_ */
