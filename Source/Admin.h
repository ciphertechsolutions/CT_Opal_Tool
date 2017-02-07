#ifndef ADMIN_H_
#define ADMIN_H_


/* If the Vista SDK is not installed, we need most of this file. */
#define TokenElevationType		18
#define TokenElevation			20

typedef enum _TOKEN_ELEVATION_TYPE {
	TokenElevationTypeDefault = 1,
	TokenElevationTypeFull,
	TokenElevationTypeLimited,
} TOKEN_ELEVATION_TYPE, *PTOKEN_ELEVATION_TYPE;

typedef struct _TOKEN_ELEVATION {
	DWORD	TokenIsElevated;
} TOKEN_ELEVATION, *PTOKEN_ELEVATION;



/* Function prototype. */
BOOL ElevateMe(void);


#endif /* ADMIN_H_ */
