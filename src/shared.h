#pragma once

#define CVAR_ARCHIVE        1   // set to cause it to be saved to vars.rc
// used for system variables, not for player
// specific configurations
#define CVAR_USERINFO       2   // sent to server on connect or change
#define CVAR_SERVERINFO     4   // sent in response to front end requests
#define CVAR_SYSTEMINFO     8   // these cvars will be duplicated on all clients
#define CVAR_INIT           16  // don't allow change from console at all,
// but can be set from the command line
#define CVAR_LATCH          32  // will only change when C code next does
// a Cvar_Get(), so it can't be changed
// without proper initialization.  modified
// will be set, even though the value hasn't
// changed yet
#define CVAR_ROM            64  // display only, cannot be set by user at all
#define CVAR_USER_CREATED   128 // created by a set command
#define CVAR_TEMP           256 // can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT          512 // can not be changed if cheats are disabled
#define CVAR_NORESTART      1024    // do not clear when a cvar_restart is issued
#define CVAR_WOLFINFO       2048    // DHM - NERVE :: Like userinfo, but for wolf multiplayer info

#define CVAR_UNSAFE         4096    // ydnar: unsafe system cvars (renderer, sound settings, anything that might cause a crash)
#define CVAR_SERVERINFO_NOUPDATE        8192    // gordon: WONT automatically send this to clients, but server browsers will see it


#define MAX_STRING_CHARS    1024    // max length of a string passed to Cmd_TokenizeString
#define MAX_STRING_TOKENS   256     // max tokens resulting from Cmd_TokenizeString
#define MAX_TOKEN_CHARS     1024    // max length of an individual token

#define MAX_RELIABLE_COMMANDS 64

#define MAX_INFO_STRING     1024
#define MAX_INFO_KEY        1024
#define MAX_INFO_VALUE      1024

#define BIG_INFO_STRING     8192    // used for system info key only
#define BIG_INFO_KEY        8192
#define BIG_INFO_VALUE      8192

#define MAX_NAME_LENGTH     36      // max length of a client name

#define MAX_QPATH 64
#define MAX_OSPATH 256

#define CONTENTS_SOLID          1       // an eye is never valid in a solid

#define CONTENTS_LIGHTGRID      4   //----(SA)	added

#define CONTENTS_LAVA           8
#define CONTENTS_SLIME          16
#define CONTENTS_WATER          32
#define CONTENTS_FOG            64


//----(SA) added
#define CONTENTS_MISSILECLIP    128 // 0x80
#define CONTENTS_ITEM           256 // 0x100
//----(SA) end

// RF, AI sight/nosight & bullet/nobullet
#define CONTENTS_AI_NOSIGHT     0x1000  // AI cannot see through this
#define CONTENTS_CLIPSHOT       0x2000  // bullets hit this
// RF, end

#define CONTENTS_MOVER          0x4000
#define CONTENTS_AREAPORTAL     0x8000

#define CONTENTS_PLAYERCLIP     0x10000
#define CONTENTS_MONSTERCLIP    0x20000

//bot specific contents types
#define CONTENTS_TELEPORTER     0x40000
#define CONTENTS_JUMPPAD        0x80000
#define CONTENTS_CLUSTERPORTAL  0x100000
#define CONTENTS_DONOTENTER     0x200000
#define CONTENTS_DONOTENTER_LARGE       0x400000

#define CONTENTS_ORIGIN         0x1000000   // removed before bsping an entity

#define CONTENTS_BODY           0x2000000   // should never be on a brush, only in game
#define CONTENTS_CORPSE         0x4000000
#define CONTENTS_DETAIL         0x8000000   // brushes not used for the bsp

#define CONTENTS_STRUCTURAL     0x10000000  // brushes used for the bsp
#define CONTENTS_TRANSLUCENT    0x20000000  // don't consume surface fragments inside
#define CONTENTS_TRIGGER        0x40000000
#define CONTENTS_NODROP         0x80000000  // don't leave bodies or items (death fog, lava)zz

enum qboolean
{
	qfalse = 0x0,
	qtrue = 0x1,
};

typedef int cvarHandle_t;
struct vmCvar_t
{
	cvarHandle_t handle;
	int modificationCount;
	float value;
	int integer;
	char string[256];
};

typedef struct {
	vmCvar_t* vmCvar;
	char* cvarName;
	char* defaultString;
	int cvarFlags;
} cvarTable_t;
#define KEYCATCH_CONSOLE        0x0001
#define KEYCATCH_UI                 0x0002
#define KEYCATCH_MESSAGE        0x0004
#define KEYCATCH_CGAME          0x0008