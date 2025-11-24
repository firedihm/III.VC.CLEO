#pragma once

// Script parameters are compiled with byte prefix that denotes their data type...
struct ScriptParamType
{
        enum : char {
                PARAM_TYPE_END_OF_PARAMS = 0,
                PARAM_TYPE_INT32 = 1,
                PARAM_TYPE_GVAR = 2,
                PARAM_TYPE_LVAR = 3,
                PARAM_TYPE_INT8 = 4,
                PARAM_TYPE_INT16 = 5,
                PARAM_TYPE_FLOAT = 6,
                PARAM_TYPE_STRING = 14
        } type : 7;
        bool processed : 1; // strings are compiled as (size byte + char arr); we convert them to c-string at runtime
};

// ...but ScriptParams array, the virtual call stack, is type-agnostic; so we'll treat it's elements as union
union ScriptParam
{
        int nVar;
        float fVar;
        char* szVar;
        void* pVar;
};

/*
    Script is being read while it's opcodes return OR_CONTINUE; like
            while (!ProcessOneCommand())
                    ;
            return;

    Opcodes that yield script's execution will return OR_TERMINATE.
    Some opcodes return OR_UNDEFINED in unexpected cases, but this value is never processed by game (cf. snippet above).
*/
enum eOpcodeResult : char
{
        OR_CONTINUE = 0,
        OR_TERMINATE = 1,
        OR_UNDEFINED = -1
};

// for models and objects loaded from scripts
struct tUsedObject
{
        char name[24];
        int index;
};

struct CVector
{
        float x, y, z;
};

struct CRect
{
        float left, bottom, right, top;
};

struct CRGBA
{
        uchar r, g, b, a;
};

struct CSprite2d
{
        void* m_pTexture; // RwTexture*
};

// stores settings for script-based string drawing, e.g. 033E: DISPLAY_TEXT
struct intro_text_line
{
        float m_fScaleX;
        float m_fScaleY;
        CRGBA m_sColor;
        bool m_bJustify;
        bool m_bCentered;
        bool m_bBackground;
        bool m_bBackgroundOnly;
        float m_fWrapX;
        float m_fCenterSize;
        CRGBA m_sBackgroundColor;
        bool m_bTextProportional;
        bool m_bTextBeforeFade;
        bool m_bRightJustify;
        int32 m_nFont;
        float m_fAtX;
        float m_fAtY;
        wchar_t text[500]; // 100 in VC, 500 in III
};

// stores settings for script-based rectangle drawing, e.g. 038E: DRAW_RECT
struct intro_script_rectangle 
{
        bool m_bIsUsed;
        bool m_bBeforeFade;
        short m_nTextureId;
        CRect m_sRect;
        CRGBA m_sColor;
};

// this was template in original: CPool<T>; void* was T*
struct CPool
{
        void* m_entries;
        uchar* m_flags;
        int m_size;
        int m_allocPtr;
};

struct bVehicleFlags
{
        uint8_t bIsLawEnforcer : 1; // Is this guy chasing the player at the moment
        uint8_t bIsAmbulanceOnDuty : 1; // Ambulance trying to get to an accident
        uint8_t bIsFireTruckOnDuty : 1; // Firetruck trying to get to a fire
        uint8_t bIsLocked : 1; // Is this guy locked by the script (cannot be removed)
        uint8_t bEngineOn : 1; // For sound purposes. Parked cars have their engines switched off (so do destroyed cars)
        uint8_t bIsHandbrakeOn : 1; // How's the handbrake doing ?
        uint8_t bLightsOn : 1; // Are the lights switched on ?
        uint8_t bFreebies : 1; // Any freebies left in this vehicle ?
};
