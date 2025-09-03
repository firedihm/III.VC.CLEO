#pragma once

constexpr int HELP_MSG_LENGTH = 256;
constexpr int INTRO_TEXT_LENGTH = 500; // 100 in VC, 500 in III

/*
    Opcode parameters are compiled with different sizes depending on their data type. Before each parameter except text labels 
    there is a one byte prefix that denotes it's data type, so script parser knows how many bytes it needs to read.

    Once a parameter has been parsed and extracted from script, it is copied to game::ScriptParams array which is type-agnostic: 
    it is an array of union.
*/
union ScriptParam
{
        enum class Type : char {
                EOP, // terminator for variadic opcodes' parameters
                Int32,
                GVar, // statically allocated in main.scm
                LVar, // script's loval_vars_
                Int8,
                Int16,
                Float,
                StringUnproc = 14, // strings are compiled as one byte length prefix and char array; we convert them to c-string at runtime
                String
        };

        int nVar;
        float fVar;
        char* szVar;
        void* pVar;
};

/*
    Script is being read while it's opcodes return OR_CONTINUE:
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
        int m_nFont;
        float m_fAtX;
        float m_fAtY;
        wchar_t text[INTRO_TEXT_LENGTH];
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
