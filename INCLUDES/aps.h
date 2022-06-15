
#define APS_PORT  "ApplicationPreferencesServer"

/* APS_OK is something but not 0 */
#define APS_OK      1
#define APS_ERROR   0

#define APS_MAXUSERNAME 32   /* including the null terminating char */

struct APSNode {
    struct  APSNode *Succ;      /* Pointer to next (successor) */
    struct  APSNode *Pred;      /* Pointer to previous (predecessor) */
    UBYTE           Type;
    BYTE            Pri;             /* Priority, for sorting */
    UBYTE           *Name;           /* ID string, null terminated */
    UBYTE           NameBuf[32];
    ULONG           PrefsLength;
    APTR            Prefs;
    };


struct APSHandle
    {
    /* Used for sending commands to the APS server */
    struct IOStdReq *Req;

    /* The client application's port is created and deleted by APS */
    struct MsgPort  *ClientPort;

    /* The developer does what he wants with this */
    APTR            ClientData;

    /* Filled at registration time, and updated depending on received
     * events. Can be an empty string.
     */
    UBYTE           UserName[APS_MAXUSERNAME];

    /* List of block allocated by OpenPrefs(), where the preferences
     * datas are stored.
     */
    struct MinList  PrefsList;

    /* DupLock() of the lock on the directory of the APServer.
     * Can be NULL if the APServer is not running.
     */
    ULONG           APSLock;

    /*-------------------------------------------------------------*/
    /*-------- client callbacks for events from the server --------*/

    /* The APS server wants to stop itself. It will stop after all
     * applications processing.
     */
    void            (*ShutDown)();

    /* A new user has been. The application perhaps wants to know.
     */
    void            (*UserAdded)();

    /* The active user has changed. The application perhaps has to
     * act upon this.
     * UserName[] contains the new active user name.
     */
    void            (*UserChanged)();

    /* If the deleted user was the active one, applications receive
     * this event. There is no more active user. The next active user
     * will be set when executing (*UserChanged)().
     * UserName[] contains 0 as first char (null C string).
     */
    void            (*UserDeleted)();
    };


/*---- Events sent by the application ---------------*/

#define APSCMD_REPLIED          0x8000

#define APSCMD_INVALID          CMD_NONSTD+0
#define APSCMD_REGISTERAPPL     CMD_NONSTD+1
#define APSCMD_UNREGISTERAPPL   CMD_NONSTD+2
#define APSCMD_OPENPREFS        CMD_NONSTD+3
#define APSCMD_SAVEPREFS        CMD_NONSTD+4
#define APSCMD_CLOSEPREFS       CMD_NONSTD+5
#define APSCMD_ADDUSERNAME      CMD_NONSTD+6
#define APSCMD_CHANGEUSERNAME   CMD_NONSTD+7
#define APSCMD_DELETEUSERNAME   CMD_NONSTD+8
#define APSCMD_SAVEPREFSENV     CMD_NONSTD+9


/*---- Events received by the application -----------*/

#define APSSRV_EVENT            0x4000

#define APSEVT_INVALID          0x4000
#define APSEVT_SHUTDOWN         0x4001
#define APSEVT_USERADDED        0x4002
#define APSEVT_USERCHANGED      0x4003
#define APSEVT_USERDELETED      0x4004
