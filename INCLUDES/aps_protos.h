ULONG APS_IsAPSHere( void );
struct APSHandle *APS_RegisterAppl( UBYTE *, struct Image *, void *, void *, void *, void *, APTR );
void APS_UnregisterAppl( struct APSHandle * );
APTR APS_OpenPrefs( struct APSHandle *, UBYTE *, UBYTE *, ULONG, ULONG, APTR, ULONG );
ULONG APS_SavePrefs( struct APSHandle *, UBYTE *, UBYTE *, UBYTE *, LONG * );
ULONG APS_SaveEnvPrefs( struct APSHandle *, UBYTE *, UBYTE *, LONG * );
void APS_ClosePrefs( struct APSHandle *, UBYTE * );
void APS_ManageEvents( struct APSHandle * );
