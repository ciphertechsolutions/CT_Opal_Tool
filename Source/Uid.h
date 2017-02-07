#ifndef UID_H_
#define UID_H_


/* The TCG UID structure. */
typedef struct tagTCGUIDD {
	BYTE	Uid[8];
	BOOL	IsClass;
	LPSTR	Description;
} TCGUID, *LPTCGUID;


/* TCG Defined UIDs/ */
extern TCGUID	NULLUID;
extern TCGUID	SP_THIS;
extern TCGUID	SM;
extern TCGUID	SM_PROPERTIES;
extern TCGUID	SM_STARTSESSION;
extern TCGUID	SM_SYNCSESSION;
extern TCGUID	SM_STARTTRUSTEDSESSION;
extern TCGUID	SM_SYNCTRUSTEDSESSION;
extern TCGUID	SM_CLOSESESSION;
extern TCGUID	TABLE_TABLE;
extern TCGUID	TABLE_SPINFO;
extern TCGUID	TABLE_SPTEMPLATES;
extern TCGUID	TABLE_COLUMN;
extern TCGUID	TABLE_TYPE;
extern TCGUID	TABLE_METHODID;
extern TCGUID	TABLE_ACCESSCONTROL;
extern TCGUID	TABLE_ACE;
extern TCGUID	TABLE_AUTHORITY;
extern TCGUID	TABLE_CERTIFICATES;
extern TCGUID	TABLE_C_PIN;
extern TCGUID	TABLE_C_RSA_1024;
extern TCGUID	TABLE_C_RSA_2048;
extern TCGUID	TABLE_C_AES_128;
extern TCGUID	TABLE_C_AES_256;
extern TCGUID	TABLE_C_EC_160;
extern TCGUID	TABLE_C_EC_192;
extern TCGUID	TABLE_C_EC_224;
extern TCGUID	TABLE_C_EC_256;
extern TCGUID	TABLE_C_EC_384;
extern TCGUID	TABLE_C_EC_521;
extern TCGUID	TABLE_C_EC_163;
extern TCGUID	TABLE_C_EC_233;
extern TCGUID	TABLE_C_EC_283;
extern TCGUID	TABLE_C_HMAC_160;
extern TCGUID	TABLE_C_HMAC_256;
extern TCGUID	TABLE_C_HMAC_384;
extern TCGUID	TABLE_C_HMAC_512;
extern TCGUID	TABLE_SECRETPROTECT;
extern TCGUID	TABLE_TPERINFO;
extern TCGUID	TABLE_CRYPTOSUITE;
extern TCGUID	TABLE_TEMPLATE;
extern TCGUID	TABLE_SP;
extern TCGUID	TABLE_CLOCKTIME;
extern TCGUID	TABLE_H_SHA_1;
extern TCGUID	TABLE_H_SHA_256;
extern TCGUID	TABLE_H_SHA_384;
extern TCGUID	TABLE_H_SHA_512;
extern TCGUID	TABLE_LOCKINGINFO;
extern TCGUID	TABLE_LOCKING;
extern TCGUID	TABLE_MBRCONTROL;
extern TCGUID	TABLE_MBR;
extern TCGUID	TABLE_K_AES_128;
extern TCGUID	TABLE_K_AES_256;
extern TCGUID	TABLE_LOG;
extern TCGUID	TABLE_LOGLIST;
extern TCGUID	TABLE_RESTRICTEDCOMMANDS;
extern TCGUID	TABLE_DATASTORE;
extern TCGUID	SPINFO;
extern TCGUID	SPTEMPLATE_BASE;
extern TCGUID	SPTEMPLATE_ADMIN;
extern TCGUID	SPTEMPLATE_CLOCK;
extern TCGUID	SPTEMPLATE_CRYPTO;
extern TCGUID	SPTEMPLATE_LOG;
extern TCGUID	SPTEMPLATE_LOCKING;
extern TCGUID	SPTEMPLATE_INTERFACECONTROL;
extern TCGUID	METHOD_DELETESP;
extern TCGUID	METHOD_CREATETABLE;
extern TCGUID	METHOD_DELETE;
extern TCGUID	METHOD_CREATEROW;
extern TCGUID	METHOD_DELETEROW;
extern TCGUID	METHOD_NEXT;
extern TCGUID	METHOD_GETFREESPACE;
extern TCGUID	METHOD_GETFREEROWS;
extern TCGUID	METHOD_DELETEMETHOD;
extern TCGUID	METHOD_GETACL;
extern TCGUID	METHOD_ADDACE;
extern TCGUID	METHOD_REMOVEACE;
extern TCGUID	METHOD_GENKEY;
extern TCGUID	METHOD_REVERTSP;
extern TCGUID	METHOD_GETPACKAGE;
extern TCGUID	METHOD_SETPACKAGE;
extern TCGUID	METHOD_GET;
extern TCGUID	METHOD_SET;
extern TCGUID	METHOD_AUTHENTICATE;
extern TCGUID	METHOD_ISSUESP;
extern TCGUID	METHOD_OPAL_REVERT;
extern TCGUID	METHOD_OPAL_ACTIVATE;
extern TCGUID	METHOD_GETCLOCK;
extern TCGUID	METHOD_RESETCLOCK;
extern TCGUID	METHOD_SETCLOCKHIGH;
extern TCGUID	METHOD_SETLAGHIGH;
extern TCGUID	METHOD_SETCLOCKLOW;
extern TCGUID	METHOD_SETLAGLOW;
extern TCGUID	METHOD_INCREMENTCOUNTER;
extern TCGUID	METHOD_RANDOM;
extern TCGUID	METHOD_SALT;
extern TCGUID	METHOD_DECRYPTINIT;
extern TCGUID	METHOD_DECRYPT;
extern TCGUID	METHOD_DECRYPTFINALIZE;
extern TCGUID	METHOD_ENCRYPTINIT;
extern TCGUID	METHOD_ENCRYPT;
extern TCGUID	METHOD_ENCRYPTFINALIZE;
extern TCGUID	METHOD_HMACINIT;
extern TCGUID	METHOD_HMAC;
extern TCGUID	METHOD_HMACFINALIZE;
extern TCGUID	METHOD_HASHINIT;
extern TCGUID	METHOD_HASH;
extern TCGUID	METHOD_HASHFINALIZE;
extern TCGUID	METHOD_SIGN;
extern TCGUID	METHOD_VERIFY;
extern TCGUID	METHOD_XOR;
extern TCGUID	METHOD_ERASE;
extern TCGUID	METHOD_ADDLOG;
extern TCGUID	METHOD_CREATELOG;
extern TCGUID	METHOD_CLEARLOG;
extern TCGUID	METHOD_FLUSHLOG;
extern TCGUID	ACE_ANYBODY;
extern TCGUID	ACE_ADMIN;
extern TCGUID	ACE_MAKERS;
extern TCGUID	ACE_SID;
extern TCGUID	ACE_CPINSIDGETNOPIN;
extern TCGUID	ACE_CPINSIDSETPIN;
extern TCGUID	ACE_CPINMSIDGETPIN;
extern TCGUID	ACE_SIDSETMAKERS;
extern TCGUID	ACE_MAKERSSETENABLED;
extern TCGUID	ACE_SPSID;
extern TCGUID	ACE_ACEGETALL;
extern TCGUID	ACE_ACESETBOOLEANEXPRESSION;
extern TCGUID	ACE_CPINADMINSGETALLNOPIN;
extern TCGUID	ACE_CPINADMINSSETPIN;
extern TCGUID	ACE_CPINUSERSETPIN;
extern TCGUID	ACE_KAES128GLOBALRANGEGENKEY;
extern TCGUID	ACE_KAES128RANGEGENKEY;
extern TCGUID	ACE_KAES256GLOBALRANGEGENKEY;
extern TCGUID	ACE_KAES256RANGEGENKEY;
extern TCGUID	ACE_KAESMODE;
extern TCGUID	ACE_LOCKINGGLOBALRANGEGETRANGESTARTTOACTIVEKEY;
extern TCGUID	ACE_LOCKINGRANGEGETRANGESTARTTOACTIVEKEY;
extern TCGUID	ACE_LOCKINGGLOBALRANGESETRDLOCKED;
extern TCGUID	ACE_LOCKINGRANGESETRDLOCKED;
extern TCGUID	ACE_LOCKINGGLOBALRANGESETWRLOCKED;
extern TCGUID	ACE_LOCKINGRANGESETWRLOCKED;
extern TCGUID	ACE_LOCKINGGLBLRNGADMINSSET;
extern TCGUID	ACE_LOCKINGADMINSRANGESTARTTOLOCKED;
extern TCGUID	ACE_MBRCONTROLADMINSSET;
extern TCGUID	ACE_MBRCONTROLSETDONE;
extern TCGUID	ACE_DATASTOREGETALL;
extern TCGUID	ACE_DATASTORESETALL;
extern TCGUID	AUTHORITY_ANYBODY;
extern TCGUID	AUTHORITY_ADMINS;
extern TCGUID	AUTHORITY_MAKERS;
extern TCGUID	AUTHORITY_MAKERSYMK;
extern TCGUID	AUTHORITY_MAKERPUK;
extern TCGUID	AUTHORITY_SID;
extern TCGUID	AUTHORITY_TPERSIGN;
extern TCGUID	AUTHORITY_TPEREXCH;
extern TCGUID	AUTHORITY_ADMINEXCH;
extern TCGUID	AUTHORITY_ISSUERS;
extern TCGUID	AUTHORITY_EDITORS;
extern TCGUID	AUTHORITY_DELETERS;
extern TCGUID	AUTHORITY_SERVERS;
extern TCGUID	AUTHORITY_RESERVE0;
extern TCGUID	AUTHORITY_RESERVE1;
extern TCGUID	AUTHORITY_RESERVE2;
extern TCGUID	AUTHORITY_RESERVE3;
extern TCGUID	AUTHORITY_ADMIN;
extern TCGUID	AUTHORITY_USERS;
extern TCGUID	AUTHORITY_USER;
extern TCGUID	C_PIN_SID;
extern TCGUID	C_PIN_MSID;
extern TCGUID	C_PIN_ADMIN;
extern TCGUID	C_PIN_USER;
extern TCGUID	TPERPINFO;
extern TCGUID	TEMPLATE_BASE;
extern TCGUID	TEMPLATE_ADMIN;
extern TCGUID	TEMPLATE_CLOCK;
extern TCGUID	TEMPLATE_CRYPTO;
extern TCGUID	TEMPLATE_LOG;
extern TCGUID	TEMPLATE_LOCKING;
extern TCGUID	TEMPLATE_INTERFACECONTROL;
extern TCGUID	SP_ADMIN;
extern TCGUID	SP_LOCKING;
extern TCGUID	CLOCKTIME_CLOCK;
extern TCGUID	LOCKINGINFO;
extern TCGUID	LOCKING_GLOBALRANGE;
extern TCGUID	LOCKING_RANGE;
extern TCGUID	MBRCONTROL;
extern TCGUID	K_AES_128_GLOBALRANGEKEY;
extern TCGUID	K_AES_128_RANGEKEY;
extern TCGUID	K_AES_256_GLOBALRANGEKEY;
extern TCGUID	K_AES_256_RANGEKEY;
extern TCGUID	LOGLIST_LOG;
extern TCGUID	TCGPSID;
extern TCGUID	C_PIN_PSID;
extern TCGUID	ACE_C_PIN_GET_PSID_NOPIN;


void AddTextDescriptions(LPTABLE Table);


#endif /* UID_H_ */
