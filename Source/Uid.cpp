#include"stdafx.h"
#include"Table.h"
#include"Uid.h"


/* TCG Defined UIDs. */
TCGUID	NULLUID = 											{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, FALSE, "Null Uid"};
TCGUID	SP_THIS =											{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, FALSE, "ThisSP"};
TCGUID	SM =												{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff}, FALSE, "SessionManager"};
TCGUID	SM_PROPERTIES =										{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x01}, FALSE, "Properties"};
TCGUID	SM_STARTSESSION =									{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x02}, FALSE, "StartSession"};
TCGUID	SM_SYNCSESSION =									{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03}, FALSE, "SyncSession"};
TCGUID	SM_STARTTRUSTEDSESSION =							{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x04}, FALSE, "StartTrustedSession"};
TCGUID	SM_SYNCTRUSTEDSESSION =								{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x05}, FALSE, "SyncTrustedSession"};
TCGUID	SM_CLOSESESSION =									{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x06}, FALSE, "CloseSession"};

TCGUID	TABLE_TABLE =										{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01}, FALSE, "Table"};
TCGUID	TABLE_SPINFO =										{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02}, FALSE, "SPInfo"};
TCGUID	TABLE_SPTEMPLATES =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03}, FALSE, "SPTemplates"};
TCGUID	TABLE_COLUMN =										{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04}, FALSE, "Column"};
TCGUID	TABLE_TYPE =										{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05}, FALSE, "Type"};
TCGUID	TABLE_METHODID =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06}, FALSE, "MethodID"};
TCGUID	TABLE_ACCESSCONTROL =								{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07}, FALSE, "AccessControl"};
TCGUID	TABLE_ACE =											{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08}, FALSE, "ACE"};
TCGUID	TABLE_AUTHORITY =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09}, FALSE, "Authority"};
TCGUID	TABLE_CERTIFICATES =								{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0A}, FALSE, "Certificates"};
TCGUID	TABLE_C_PIN =										{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0B}, FALSE, "C_PIN"};
TCGUID	TABLE_C_RSA_1024 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C}, FALSE, "C_RSA_1024"};
TCGUID	TABLE_C_RSA_2048 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0D}, FALSE, "C_RSA_2048"};
TCGUID	TABLE_C_AES_128 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0E}, FALSE, "C_AES_128"};
TCGUID	TABLE_C_AES_256 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0F}, FALSE, "C_AES_256"};
TCGUID	TABLE_C_EC_160 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10}, FALSE, "C_EC_160"};
TCGUID	TABLE_C_EC_192 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x11}, FALSE, "C_EC_192"};
TCGUID	TABLE_C_EC_224 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x12}, FALSE, "C_EC_224"};
TCGUID	TABLE_C_EC_256 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x13}, FALSE, "C_EC_256"};
TCGUID	TABLE_C_EC_384 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x14}, FALSE, "C_EC_384"};
TCGUID	TABLE_C_EC_521 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x15}, FALSE, "C_EC_521"};
TCGUID	TABLE_C_EC_163 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x16}, FALSE, "C_EC_163"};
TCGUID	TABLE_C_EC_233 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x17}, FALSE, "C_EC_233"};
TCGUID	TABLE_C_EC_283 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x18}, FALSE, "C_EC_283"};
TCGUID	TABLE_C_HMAC_160 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x19}, FALSE, "C_HMAC_160"};
TCGUID	TABLE_C_HMAC_256 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1A}, FALSE, "C_HMAC_256"};
TCGUID	TABLE_C_HMAC_384 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1B}, FALSE, "C_HMAC_384"};
TCGUID	TABLE_C_HMAC_512 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1C}, FALSE, "C_HMAC_512"};
TCGUID	TABLE_SECRETPROTECT =								{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1D}, FALSE, "SecretProtect"};
TCGUID	TABLE_TPERINFO =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x01}, FALSE, "TPerInfo"};
TCGUID	TABLE_CRYPTOSUITE =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x03}, FALSE, "CryptoSuite"};
TCGUID	TABLE_TEMPLATE =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x04}, FALSE, "Template"};
TCGUID	TABLE_SP =											{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x05}, FALSE, "SP"};
TCGUID	TABLE_CLOCKTIME =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x04, 0x01}, FALSE, "ClockTime"};
TCGUID	TABLE_H_SHA_1 =										{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x01}, FALSE, "H_SHA_1"};
TCGUID	TABLE_H_SHA_256 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x02}, FALSE, "H_SHA_256"};
TCGUID	TABLE_H_SHA_384 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x03}, FALSE, "H_SHA_384"};
TCGUID	TABLE_H_SHA_512 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x04}, FALSE, "H_SHA_512"};
TCGUID	TABLE_LOCKINGINFO =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x01}, FALSE, "LockingInfo"};
TCGUID	TABLE_LOCKING =										{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x02}, FALSE, "Locking"};
TCGUID	TABLE_MBRCONTROL =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x03}, FALSE, "MBRControl"};
TCGUID	TABLE_MBR =											{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x04}, FALSE, "MBR"};
TCGUID	TABLE_K_AES_128 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x05}, FALSE, "K_AES_128"};
TCGUID	TABLE_K_AES_256 =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x06}, FALSE, "K_AES_256"};
TCGUID	TABLE_LOG =											{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0A, 0x01}, FALSE, "Log"};
TCGUID	TABLE_LOGLIST =										{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0A, 0x02}, FALSE, "LogList"};
TCGUID	TABLE_RESTRICTEDCOMMANDS =							{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0C, 0x01}, FALSE, "RestrictedCommands"};
TCGUID	TABLE_DATASTORE =									{{0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10, 0x01}, FALSE, "DataStore"};

TCGUID	SPINFO =											{{0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01}, FALSE, "SPInfo"};

TCGUID	SPTEMPLATE_BASE =									{{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01}, FALSE, "Base"};
TCGUID	SPTEMPLATE_ADMIN =									{{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02}, FALSE, "Admin"};
TCGUID	SPTEMPLATE_CLOCK =									{{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03}, FALSE, "Clock"};
TCGUID	SPTEMPLATE_CRYPTO =									{{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04}, FALSE, "Crypto"};
TCGUID	SPTEMPLATE_LOG =									{{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05}, FALSE, "Log"};
TCGUID	SPTEMPLATE_LOCKING =								{{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x06}, FALSE, "Locking"};
TCGUID	SPTEMPLATE_INTERFACECONTROL =						{{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07}, FALSE, "Interface Control"};

TCGUID	METHOD_DELETESP =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01}, FALSE, "DeleteSP"};
TCGUID	METHOD_CREATETABLE =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x02}, FALSE, "CreateTable"};
TCGUID	METHOD_DELETE =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x03}, FALSE, "Delete"};
TCGUID	METHOD_CREATEROW =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x04}, FALSE, "CreateRow"};
TCGUID	METHOD_DELETEROW =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05}, FALSE, "DeleteRow"};
TCGUID	METHOD_NEXT =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x08}, FALSE, "Next"};
TCGUID	METHOD_GETFREESPACE =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x09}, FALSE, "GetFreeSpace"};
TCGUID	METHOD_GETFREEROWS =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0A}, FALSE, "GetFreeRows"};
TCGUID	METHOD_DELETEMETHOD =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0B}, FALSE, "DeleteMethod"};
TCGUID	METHOD_GETACL =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0D}, FALSE, "GetACL"};
TCGUID	METHOD_ADDACE =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0E}, FALSE, "AddACE"};
TCGUID	METHOD_REMOVEACE =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0F}, FALSE, "RemoveACE"};
TCGUID	METHOD_GENKEY =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x10}, FALSE, "GenKey"};
TCGUID	METHOD_REVERTSP =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x11}, FALSE, "RevertSP"};
TCGUID	METHOD_GETPACKAGE =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x12}, FALSE, "GetPackage"};
TCGUID	METHOD_SETPACKAGE =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x13}, FALSE, "SetPackage"};
TCGUID	METHOD_GET =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x16}, FALSE, "Get"};
TCGUID	METHOD_SET =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x17}, FALSE, "Set"};
TCGUID	METHOD_AUTHENTICATE =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x1C}, FALSE, "Authenticate"};
TCGUID	METHOD_ISSUESP =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x01}, FALSE, "IssueSP"};
TCGUID	METHOD_OPAL_REVERT =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x02}, FALSE, "Revert"};
TCGUID	METHOD_OPAL_ACTIVATE =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x03}, FALSE, "Activate"};
TCGUID	METHOD_GETCLOCK =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x01}, FALSE, "GetClock"};
TCGUID	METHOD_RESETCLOCK =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x02}, FALSE, "ResetClock"};
TCGUID	METHOD_SETCLOCKHIGH =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x03}, FALSE, "SetClockHigh"};
TCGUID	METHOD_SETLAGHIGH =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x04}, FALSE, "SetLagHigh"};
TCGUID	METHOD_SETCLOCKLOW =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x05}, FALSE, "SetClockLow"};
TCGUID	METHOD_SETLAGLOW =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x06}, FALSE, "SetLagLow"};
TCGUID	METHOD_INCREMENTCOUNTER =							{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x07}, FALSE, "IncrementCounter"};
TCGUID	METHOD_RANDOM =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x01}, FALSE, "Random"};
TCGUID	METHOD_SALT =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x02}, FALSE, "Salt"};
TCGUID	METHOD_DECRYPTINIT =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x03}, FALSE, "DecryptInit"};
TCGUID	METHOD_DECRYPT =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x04}, FALSE, "Decrypt"};
TCGUID	METHOD_DECRYPTFINALIZE =							{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x05}, FALSE, "DecryptFinalize"};
TCGUID	METHOD_ENCRYPTINIT =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x06}, FALSE, "EncryptInit"};
TCGUID	METHOD_ENCRYPT =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x07}, FALSE, "Encrypt"};
TCGUID	METHOD_ENCRYPTFINALIZE =							{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x08}, FALSE, "EncryptFinalize"};
TCGUID	METHOD_HMACINIT =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x09}, FALSE, "HMACInit"};
TCGUID	METHOD_HMAC =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x0A}, FALSE, "HMAC"};
TCGUID	METHOD_HMACFINALIZE =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x0B}, FALSE, "HMACFinalize"};
TCGUID	METHOD_HASHINIT =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x0C}, FALSE, "HashInit"};
TCGUID	METHOD_HASH =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x0D}, FALSE, "Hash"};
TCGUID	METHOD_HASHFINALIZE =								{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x0E}, FALSE, "HashFinalize"};
TCGUID	METHOD_SIGN =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x0F}, FALSE, "Sign"};
TCGUID	METHOD_VERIFY =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x10}, FALSE, "Verify"};
TCGUID	METHOD_XOR =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x11}, FALSE, "XOR"};
TCGUID	METHOD_ERASE =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x08, 0x03}, FALSE, "Erase"};
TCGUID	METHOD_ADDLOG =										{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x0A, 0x01}, FALSE, "AddLog"};
TCGUID	METHOD_CREATELOG =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x0A, 0x02}, FALSE, "CreateLog"};
TCGUID	METHOD_CLEARLOG =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x0A, 0x03}, FALSE, "ClearLog"};
TCGUID	METHOD_FLUSHLOG =									{{0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x0A, 0x04}, FALSE, "FlushLog"};

TCGUID	ACE_ANYBODY =										{{0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01}, FALSE, "ACE_Anybody"};
TCGUID	ACE_ADMIN =											{{0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x02}, FALSE, "ACE_Admin"};
TCGUID	ACE_MAKERS =										{{0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03}, FALSE, "ACE_Makers"};
TCGUID	ACE_SID =											{{0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x02, 0x01}, FALSE, "ACE_SID"};
TCGUID	ACE_CPINSIDGETNOPIN =								{{0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x8c, 0x02}, FALSE, "ACE_C_PIN_SID_Get_NOPIN"};
TCGUID	ACE_CPINSIDSETPIN =									{{0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x8c, 0x03}, FALSE, "ACE_C_PIN_SID_Set_PIN"};
TCGUID	ACE_CPINMSIDGETPIN =								{{0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x8c, 0x04}, FALSE, "ACE_C_PIN_MSID_Get_PIN"};
TCGUID	ACE_SIDSETMAKERS =									{{0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x8c, 0x05}, FALSE, "ACE_SID_SetMakers"};
TCGUID	ACE_MAKERSSETENABLED =								{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0x00, 0x01}, FALSE, "ACE_Makers_Set_Enabled"};
TCGUID	ACE_SPSID =											{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0x00, 0x02}, FALSE, "ACE_SP_SID"};
TCGUID	ACE_ACEGETALL =										{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0x80, 0x00}, FALSE, "ACE_ACE_Get_All"};
TCGUID	ACE_ACESETBOOLEANEXPRESSION =						{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0x80, 0x01}, FALSE, "ACE_ACE_Set_BooleanExpression"};
TCGUID	ACE_CPINADMINSGETALLNOPIN =							{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xa0, 0x00}, FALSE, "ACE_C_PIN_Admins_Get_All_NOPIN"};
TCGUID	ACE_CPINADMINSSETPIN =								{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xa0, 0x01}, FALSE, "ACE_C_PIN_Admins_Set_PIN"};
TCGUID	ACE_CPINUSERSETPIN =								{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xa8, 0x00}, TRUE, "ACE_C_PIN_User%d_Set_PIN"};
TCGUID	ACE_KAES128GLOBALRANGEGENKEY =						{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xb0, 0x00}, FALSE, "ACE_K_AES_128_GlobalRange_GenKey"};
TCGUID	ACE_KAES128RANGEGENKEY =							{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xb0, 0x00}, TRUE, "ACE_K_AES_128_Range%d_GenKey"};
TCGUID	ACE_KAES256GLOBALRANGEGENKEY =						{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xb8, 0x00}, FALSE, "ACE_K_AES_256_GlobalRange_GenKey"};
TCGUID	ACE_KAES256RANGEGENKEY =							{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xb8, 0x00}, TRUE, "ACE_K_AES_256_Range%d_GenKey"};
TCGUID	ACE_KAESMODE =										{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xbf, 0xff}, FALSE, "ACE_K_AES_Mode"};
TCGUID	ACE_LOCKINGGLOBALRANGEGETRANGESTARTTOACTIVEKEY =	{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xd0, 0x00}, FALSE, "ACE_Locking_GlobalRange_Get_RangeStartToActiveKey"};
TCGUID	ACE_LOCKINGRANGEGETRANGESTARTTOACTIVEKEY =			{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xd0, 0x00}, TRUE, "ACE_Locking_Range%d_Get_RangeStartToActiveKey"};
TCGUID	ACE_LOCKINGGLOBALRANGESETRDLOCKED =					{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xe0, 0x00}, FALSE, "ACE_Locking_GlobalRange_Set_RdLocked"};
TCGUID	ACE_LOCKINGRANGESETRDLOCKED =						{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xe0, 0x00}, TRUE, "ACE_Locking_Range%d_Set_RdLocked"};
TCGUID	ACE_LOCKINGGLOBALRANGESETWRLOCKED =					{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xe8, 0x00}, FALSE, "ACE_Locking_GlobalRange_Set_WrLocked"};
TCGUID	ACE_LOCKINGRANGESETWRLOCKED =						{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xe8, 0x00}, TRUE, "ACE_Locking_Range%d_Set_WrLocked"};
TCGUID	ACE_LOCKINGGLBLRNGADMINSSET =						{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xf0, 0x00}, FALSE, "ACE_Locking_GlblRng_Admins_Set"};
TCGUID	ACE_LOCKINGADMINSRANGESTARTTOLOCKED =				{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xf0, 0x01}, FALSE, "ACE_Locking_Admins_RangeStartToLocked"};
TCGUID	ACE_MBRCONTROLADMINSSET =							{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xf8, 0x00}, FALSE, "ACE_MBRControl_Admins_Set"};
TCGUID	ACE_MBRCONTROLSETDONE =								{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xf8, 0x01}, FALSE, "ACE_MBRControl_Set_Done"};
TCGUID	ACE_DATASTOREGETALL =								{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xfc, 0x00}, FALSE, "ACE_DataStore_Get_All"};
TCGUID	ACE_DATASTORESETALL =								{{0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0xfc, 0x01}, FALSE, "ACE_DataStore_Set_All"};

TCGUID	AUTHORITY_ANYBODY =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x01}, FALSE, "Anybody"};
TCGUID	AUTHORITY_ADMINS =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x02}, FALSE, "Admins"};
TCGUID	AUTHORITY_MAKERS =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x03}, FALSE, "Makers"};
TCGUID	AUTHORITY_MAKERSYMK =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x04}, FALSE, "MakerSymK"};
TCGUID	AUTHORITY_MAKERPUK =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x05}, FALSE, "MakerPuk"};
TCGUID	AUTHORITY_SID =										{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06}, FALSE, "SID"};
TCGUID	AUTHORITY_TPERSIGN =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x07}, FALSE, "TPerSign"};
TCGUID	AUTHORITY_TPEREXCH =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x08}, FALSE, "TPerExch"};
TCGUID	AUTHORITY_ADMINEXCH =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x09}, FALSE, "AdminExch"};
TCGUID	AUTHORITY_ISSUERS =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x02, 0x01}, FALSE, "Issuers"};
TCGUID	AUTHORITY_EDITORS =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x02, 0x02}, FALSE, "Editors"};
TCGUID	AUTHORITY_DELETERS =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x02, 0x03}, FALSE, "Deleters"};
TCGUID	AUTHORITY_SERVERS =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x02, 0x04}, FALSE, "Servers"};
TCGUID	AUTHORITY_RESERVE0 =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x02, 0x05}, FALSE, "Reserve0"};
TCGUID	AUTHORITY_RESERVE1 =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x02, 0x06}, FALSE, "Reserve1"};
TCGUID	AUTHORITY_RESERVE2 =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x02, 0x07}, FALSE, "Reserve2"};
TCGUID	AUTHORITY_RESERVE3 =								{{0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x02, 0x08}, FALSE, "Reserve3"};
TCGUID	AUTHORITY_ADMIN =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x01, 0x00, 0x00}, TRUE, "Admin%d"};
TCGUID	AUTHORITY_USERS =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x03, 0x00, 0x00}, FALSE, "Users"};
TCGUID	AUTHORITY_USER =									{{0x00, 0x00, 0x00, 0x09, 0x00, 0x03, 0x00, 0x00}, TRUE, "User%d"};

TCGUID	C_PIN_SID =											{{0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x01}, FALSE, "C_PIN_SID"};
TCGUID	C_PIN_MSID =										{{0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x84, 0x02}, FALSE, "C_PIN_MSID"};
TCGUID	C_PIN_ADMIN =										{{0x00, 0x00, 0x00, 0x0B, 0x00, 0x01, 0x00, 0x00}, TRUE, "C_PIN_Admin%d"};
TCGUID	C_PIN_USER =										{{0x00, 0x00, 0x00, 0x0B, 0x00, 0x03, 0x00, 0x00}, TRUE, "C_PIN_User%d"};

TCGUID	TPERPINFO =											{{0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x01}, FALSE, "TPerInfo"};

TCGUID	TEMPLATE_BASE =										{{0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x01}, FALSE, "Base"};
TCGUID	TEMPLATE_ADMIN =									{{0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x02}, FALSE, "Admin"};
TCGUID	TEMPLATE_CLOCK =									{{0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x03}, FALSE, "Clock"};
TCGUID	TEMPLATE_CRYPTO =									{{0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x04}, FALSE, "Crypto"};
TCGUID	TEMPLATE_LOG =										{{0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x05}, FALSE, "Log"};
TCGUID	TEMPLATE_LOCKING =									{{0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x06}, FALSE, "Locking"};
TCGUID	TEMPLATE_INTERFACECONTROL =							{{0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x07}, FALSE, "InterfaceControl"};

TCGUID	SP_ADMIN =											{{0x00, 0x00, 0x02, 0x05, 0x00, 0x00, 0x00, 0x01}, FALSE, "Admin"};
TCGUID	SP_LOCKING =										{{0x00, 0x00, 0x02, 0x05, 0x00, 0x00, 0x00, 0x02}, FALSE, "Locking"};

TCGUID	CLOCKTIME_CLOCK =									{{0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x01}, FALSE, "Clock"};

TCGUID	LOCKINGINFO =										{{0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x01}, FALSE, "LockingInfo"};

TCGUID	LOCKING_GLOBALRANGE =								{{0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00, 0x01}, FALSE, "Locking_GlobalRange"};
TCGUID	LOCKING_RANGE =										{{0x00, 0x00, 0x08, 0x02, 0x00, 0x03, 0x00, 0x00}, TRUE, "Locking_Range%d"};

TCGUID	MBRCONTROL =										{{0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00, 0x01}, FALSE, "MBRControl"};

TCGUID	K_AES_128_GLOBALRANGEKEY =							{{0x00, 0x00, 0x08, 0x05, 0x00, 0x00, 0x00, 0x01}, FALSE, "K_AES_128_GlobalRange_Key"};
TCGUID	K_AES_128_RANGEKEY =								{{0x00, 0x00, 0x08, 0x05, 0x00, 0x03, 0x00, 0x00}, TRUE, "K_AES_128_Range%d_Key"};

TCGUID	K_AES_256_GLOBALRANGEKEY =							{{0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x01}, FALSE, "K_AES_256_GlobalRange_Key"};
TCGUID	K_AES_256_RANGEKEY =								{{0x00, 0x00, 0x08, 0x06, 0x00, 0x03, 0x00, 0x00}, TRUE, "K_AES_256_Range%d_Key"};

TCGUID	LOGLIST_LOG =										{{0x00, 0x00, 0x0A, 0x02, 0x00, 0x00, 0x00, 0x01}, FALSE, "Log"};

/* Additional UIDs from the PSID specification. */
/* Note: Windows already defines a PSID, so we have to call it something different. */
TCGUID	TCGPSID =											{{0x00, 0x00, 0x00, 0x09, 0x00, 0x01, 0xff, 0x01}, FALSE, "PSID"};
TCGUID	C_PIN_PSID =										{{0x00, 0x00, 0x00, 0x0B, 0x00, 0x01, 0xff, 0x01}, FALSE, "C_PIN_PSID"};
TCGUID	ACE_C_PIN_GET_PSID_NOPIN =							{{0x00, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0xe1}, FALSE, "ACE_C_PIN_Get_PSID_NoPIN"};

/* The list of all the UIDs above. */
static LPTCGUID UidList[] = {
	&NULLUID,
	&SP_THIS,
	&SM,
	&SM_PROPERTIES,
	&SM_STARTSESSION,
	&SM_SYNCSESSION,
	&SM_STARTTRUSTEDSESSION,
	&SM_SYNCTRUSTEDSESSION,
	&SM_CLOSESESSION,
	&TABLE_TABLE,
	&TABLE_SPINFO,
	&TABLE_SPTEMPLATES,
	&TABLE_COLUMN,
	&TABLE_TYPE,
	&TABLE_METHODID,
	&TABLE_ACCESSCONTROL,
	&TABLE_ACE,
	&TABLE_AUTHORITY,
	&TABLE_CERTIFICATES,
	&TABLE_C_PIN,
	&TABLE_C_RSA_1024,
	&TABLE_C_RSA_2048,
	&TABLE_C_AES_128,
	&TABLE_C_AES_256,
	&TABLE_C_EC_160,
	&TABLE_C_EC_192,
	&TABLE_C_EC_224,
	&TABLE_C_EC_256,
	&TABLE_C_EC_384,
	&TABLE_C_EC_521,
	&TABLE_C_EC_163,
	&TABLE_C_EC_233,
	&TABLE_C_EC_283,
	&TABLE_C_HMAC_160,
	&TABLE_C_HMAC_256,
	&TABLE_C_HMAC_384,
	&TABLE_C_HMAC_512,
	&TABLE_SECRETPROTECT,
	&TABLE_TPERINFO,
	&TABLE_CRYPTOSUITE,
	&TABLE_TEMPLATE,
	&TABLE_SP,
	&TABLE_CLOCKTIME,
	&TABLE_H_SHA_1,
	&TABLE_H_SHA_256,
	&TABLE_H_SHA_384,
	&TABLE_H_SHA_512,
	&TABLE_LOCKINGINFO,
	&TABLE_LOCKING,
	&TABLE_MBRCONTROL,
	&TABLE_MBR,
	&TABLE_K_AES_128,
	&TABLE_K_AES_256,
	&TABLE_LOG,
	&TABLE_LOGLIST,
	&TABLE_RESTRICTEDCOMMANDS,
	&TABLE_DATASTORE,
	&SPINFO,
	&SPTEMPLATE_BASE,
	&SPTEMPLATE_ADMIN,
	&SPTEMPLATE_CLOCK,
	&SPTEMPLATE_CRYPTO,
	&SPTEMPLATE_LOG,
	&SPTEMPLATE_LOCKING,
	&SPTEMPLATE_INTERFACECONTROL,
	&METHOD_DELETESP,
	&METHOD_CREATETABLE,
	&METHOD_DELETE,
	&METHOD_CREATEROW,
	&METHOD_DELETEROW,
	&METHOD_NEXT,
	&METHOD_GETFREESPACE,
	&METHOD_GETFREEROWS,
	&METHOD_DELETEMETHOD,
	&METHOD_GETACL,
	&METHOD_ADDACE,
	&METHOD_REMOVEACE,
	&METHOD_GENKEY,
	&METHOD_REVERTSP,
	&METHOD_GETPACKAGE,
	&METHOD_SETPACKAGE,
	&METHOD_GET,
	&METHOD_SET,
	&METHOD_AUTHENTICATE,
	&METHOD_ISSUESP,
	&METHOD_OPAL_REVERT,
	&METHOD_OPAL_ACTIVATE,
	&METHOD_GETCLOCK,
	&METHOD_RESETCLOCK,
	&METHOD_SETCLOCKHIGH,
	&METHOD_SETLAGHIGH,
	&METHOD_SETCLOCKLOW,
	&METHOD_SETLAGLOW,
	&METHOD_INCREMENTCOUNTER,
	&METHOD_RANDOM,
	&METHOD_SALT,
	&METHOD_DECRYPTINIT,
	&METHOD_DECRYPT,
	&METHOD_DECRYPTFINALIZE,
	&METHOD_ENCRYPTINIT,
	&METHOD_ENCRYPT,
	&METHOD_ENCRYPTFINALIZE,
	&METHOD_HMACINIT,
	&METHOD_HMAC,
	&METHOD_HMACFINALIZE,
	&METHOD_HASHINIT,
	&METHOD_HASH,
	&METHOD_HASHFINALIZE,
	&METHOD_SIGN,
	&METHOD_VERIFY,
	&METHOD_XOR,
	&METHOD_ERASE,
	&METHOD_ADDLOG,
	&METHOD_CREATELOG,
	&METHOD_CLEARLOG,
	&METHOD_FLUSHLOG,
	&ACE_ANYBODY,
	&ACE_ADMIN,
	&ACE_MAKERS,
	&ACE_SID,
	&ACE_CPINSIDGETNOPIN,
	&ACE_CPINSIDSETPIN,
	&ACE_CPINMSIDGETPIN,
	&ACE_SIDSETMAKERS,
	&ACE_MAKERSSETENABLED,
	&ACE_SPSID,
	&ACE_ACEGETALL,
	&ACE_ACESETBOOLEANEXPRESSION,
	&ACE_CPINADMINSGETALLNOPIN,
	&ACE_CPINADMINSSETPIN,
	&ACE_CPINUSERSETPIN,
	&ACE_KAES128GLOBALRANGEGENKEY,
	&ACE_KAES128RANGEGENKEY,
	&ACE_KAES256GLOBALRANGEGENKEY,
	&ACE_KAES256RANGEGENKEY,
	&ACE_KAESMODE,
	&ACE_LOCKINGGLOBALRANGEGETRANGESTARTTOACTIVEKEY,
	&ACE_LOCKINGRANGEGETRANGESTARTTOACTIVEKEY,
	&ACE_LOCKINGGLOBALRANGESETRDLOCKED,
	&ACE_LOCKINGRANGESETRDLOCKED,
	&ACE_LOCKINGGLOBALRANGESETWRLOCKED,
	&ACE_LOCKINGRANGESETWRLOCKED,
	&ACE_LOCKINGGLBLRNGADMINSSET,
	&ACE_LOCKINGADMINSRANGESTARTTOLOCKED,
	&ACE_MBRCONTROLADMINSSET,
	&ACE_MBRCONTROLSETDONE,
	&ACE_DATASTOREGETALL,
	&ACE_DATASTORESETALL,
	&AUTHORITY_ANYBODY,
	&AUTHORITY_ADMINS,
	&AUTHORITY_MAKERS,
	&AUTHORITY_MAKERSYMK,
	&AUTHORITY_MAKERPUK,
	&AUTHORITY_SID,
	&AUTHORITY_TPERSIGN,
	&AUTHORITY_TPEREXCH,
	&AUTHORITY_ADMINEXCH,
	&AUTHORITY_ISSUERS,
	&AUTHORITY_EDITORS,
	&AUTHORITY_DELETERS,
	&AUTHORITY_SERVERS,
	&AUTHORITY_RESERVE0,
	&AUTHORITY_RESERVE1,
	&AUTHORITY_RESERVE2,
	&AUTHORITY_RESERVE3,
	&AUTHORITY_ADMIN,
	&AUTHORITY_USERS,
	&AUTHORITY_USER,
	&C_PIN_SID,
	&C_PIN_MSID,
	&C_PIN_ADMIN,
	&C_PIN_USER,
	&TPERPINFO,
	&TEMPLATE_BASE,
	&TEMPLATE_ADMIN,
	&TEMPLATE_CLOCK,
	&TEMPLATE_CRYPTO,
	&TEMPLATE_LOG,
	&TEMPLATE_LOCKING,
	&TEMPLATE_INTERFACECONTROL,
	&SP_ADMIN,
	&SP_LOCKING,
	&CLOCKTIME_CLOCK,
	&LOCKINGINFO,
	&LOCKING_GLOBALRANGE,
	&LOCKING_RANGE,
	&MBRCONTROL,
	&K_AES_128_GLOBALRANGEKEY,
	&K_AES_128_RANGEKEY,
	&K_AES_256_GLOBALRANGEKEY,
	&K_AES_256_RANGEKEY,
	&LOGLIST_LOG,
	&TCGPSID,
	&C_PIN_PSID,
	&ACE_C_PIN_GET_PSID_NOPIN,
};


/*
 * For a known UID, adds the text decription to the table if it's not provided.
 */
void AddTextDescriptions(LPTABLE Table)
{
	LPTABLECELL	Index, Next;
	LPSTR		ToAdd;
	char		InputString[100];
	int			Rows;
	int			i, j;

	/* Get the number of rows in the table. */
	Rows = GetRows(Table);

	/* Search table for each column 0 entry. */
	for(j=0; j<Rows; j++) {
		/* Get the column 0 and 1 entries. */
		Next = GetTableCell(Table, j, 0);
		Index = GetTableCell(Table, j, 1);

		/* If there is no corresponding column 1 entry, then add one. */
		ToAdd = "Unknown";
		if(Index == NULL) {
			for(i=0; i<sizeof(UidList)/sizeof(UidList[0]); i++) {
				if(UidList[i]->IsClass) {
					if(memcmp(Next->Bytes, UidList[i]->Uid, 7) == 0) {
						wsprintfA(InputString, UidList[i]->Description, Next->Bytes[7]);
						ToAdd = InputString;
						break;
					}
				} else {
					if(memcmp(Next->Bytes, UidList[i]->Uid, 8) == 0) {
						ToAdd = UidList[i]->Description;
						break;
					}
				}
			}
			AddCell(Table, j, 1, lstrlenA(ToAdd), (LPBYTE)ToAdd);
		}
	}
}
