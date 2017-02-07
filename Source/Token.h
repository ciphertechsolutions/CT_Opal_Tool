#ifndef TOKEN_H_
#define TOKEN_H_

/* Tokens used in subpackets. */
#define	TCG_TOKEN_TINYATOM			0x00
#define	TCG_TOKEN_TINYSIGNEDATOM	0x40
#define	TCG_TOKEN_SHORTATOM			0x80
#define	TCG_TOKEN_SHORTSIGNEDATOM	0x90
#define	TCG_TOKEN_SHORTBYTESATOM	0xa0
#define	TCG_TOKEN_MEDIUMATOM		0xc0
#define	TCG_TOKEN_MEDIUMSIGNEDATOM	0xc8
#define	TCG_TOKEN_MEDIUMBYTESATOM	0xd0
#define	TCG_TOKEN_LONGATOM			0xe0
#define	TCG_TOKEN_LONGSIGNEDATOM	0xe1
#define	TCG_TOKEN_LONGBYTESATOM		0xe2
#define	TCG_TOKEN_STARTLIST			0xf0
#define	TCG_TOKEN_ENDLIST			0xf1
#define	TCG_TOKEN_STARTNAME			0xf2
#define	TCG_TOKEN_ENDNAME			0xf3
#define	TCG_TOKEN_CALL				0xf8
#define	TCG_TOKEN_ENDDATA			0xf9
#define	TCG_TOKEN_ENDSESSION		0xfa
#define	TCG_TOKEN_STARTTRANSACTION	0xfb
#define	TCG_TOKEN_ENDTRANSACTION	0xfc
#define	TCG_TOKEN_EMPTY				0xff
/* My own internal token types. */
#define	TCG_TOKEN_TYPE_INT			0x00
#define TCG_TOKEN_TYPE_BYTES		0xa0


/* Internal structure used for parsing tokens. */
typedef struct tagInternalToken {
	BYTE		Type;			/* Token type, integer, bytes or control. */
	int			Integer;		/* If an integer, this holds the token value. If bytes, the number of bytes. */
	LPBYTE		Bytes;			/* If bytes, this points to them. */
} TOKEN, *LPTOKEN;


/* Internal structure used for parsing tokens. */
typedef struct tagTokens {
	int			Count;			/* Count of all tokens. */
	TOKEN		Tokens[1];		/* List of tokens. */
} TOKENS, *LPTOKENS;


/* Function prototypes. */
void AddSimpleToken(LPTCGFULLPACKET ComPacket, BYTE Token);
void AddTokenBytes(LPTCGFULLPACKET ComPacket, LPBYTE Bytes, int Size);
void AddTokenSignedInt(LPTCGFULLPACKET ComPacket, int Integer);
void AddTokenUnsignedInt(LPTCGFULLPACKET ComPacket, QWORD Integer);
void AddTokenEndParameters(LPTCGFULLPACKET ComPacket, BYTE Status);
void AddTokenCall(LPTCGFULLPACKET ComPacket, LPBYTE InvokingId, LPBYTE MethodId);
void AddTokenCellBlock(LPTCGFULLPACKET ComPacket, LPBYTE Table, LPBYTE StartRowUid, int StartRowInt, int EndRow, int StartColumn, int EndColumn);
LPTOKENS ParseResponse(LPTCGFULLPACKET ComPacket);
BOOL CheckResponseForClose(LPTCGFULLPACKET ComPacket);
BYTE GetResponseResultCode(LPTCGFULLPACKET ComPacket);
LPBYTE GetResponseBytes(LPTCGFULLPACKET ComPacket, LPDWORD Size);


#endif /* TOKEN_H_ */
