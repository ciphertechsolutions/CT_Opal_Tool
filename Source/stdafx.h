#pragma once
#pragma comment(lib, "comctl32.lib")

#include "targetver.h"

#define STRICT
#include <windows.h>
#include <assert.h>
#include <commctrl.h>
#include <shellapi.h>
#include <setupapi.h>
#include <stddef.h>
#include "Passthrough.h"
#include "Storage.h"

typedef unsigned __int64	QWORD;
typedef QWORD				*LPQWORD;
#define	_T(x)				(x)

#define MAXSUPPORTEDDRIVE	128

#define	BE_TO_LE_16(x)		((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define LE_TO_BE_16(x)		BE_TO_LE_16(x)
#define BE_TO_LE_32(x)		((((x) >> 24) & 0xff) | (((x) >> 8) & 0xff00) | (((x) & 0xff00) << 8) | (((x) & 0xff) << 24))
#define LE_TO_BE_32(x)		BE_TO_LE_32(x)
#define BE_TO_LE_64(x)		((((x) >> 56) & 0xff) | (((x) >> 40) & 0xff00) | (((x) >> 24) & 0xff0000) | (((x) >> 8) & 0xff000000) | (((x) & 0xff000000) << 8) | (((x) & 0xff0000) << 24) | (((x) & 0xff00) << 40) | (((x) & 0xff) << 56))
#define LE_TO_BE_64(x)		BE_TO_LE_64(x)
