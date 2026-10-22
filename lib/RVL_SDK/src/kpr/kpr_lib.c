#include <revolution/kpr.h>

#include <revolution/os.h>
#include <revolution/types.h>

#include <wchar.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

typedef wchar_t wint_t;
#define WEOF ((wint_t)0xFFFF)

#undef NULL
#define NULL ((void*)0)

#define OSError_Line(line_, ...) OSPanic(__FILE__, line_, __VA_ARGS__)
#define OSAssert_Line(line_, exp_) ((void)0)
#define OSAssertMessage_Line(line_, exp_, ...) ((void)0)

#define RVL_SDK_LIB_VERSION_STRING(lib_) "<< RVL_SDK - " #lib_ " 	release build: Aug  8 2007 02:10:51 (0x4199_60831) >>"

/*******************************************************************************
 * types
 */

typedef void KPRProcessDeadKeysFunc(KPRQueue *queue);
typedef void KPRProcessRomajiFunc(KPRQueue *queue);

struct RmJpRule
{
	u8				rmLen;	// size 0x01, offset 0x00
	unsigned char	rm[4];	// size 0x04, offset 0x01	// why unsigned?
	u8				jpLen;	// size 0x01, offset 0x05
	wchar_t			jp[2];	// size 0x04, offset 0x06
}; // size 0x0a

/*******************************************************************************
 * local function declarations
 */

static inline BOOL isConsonant(wchar_t ch)
{
	if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u')
		return FALSE;

	return TRUE;
}

static BOOL KPRProcessAltKeypad(KPRQueue *queue, wchar_t ch);
static void KPRProcessDeadKeys(KPRQueue *queue);
static void KPRProcessRomaji(KPRQueue *queue);

/*******************************************************************************
 * variables
 */

// .rodata
static wchar_t const kprDeadKeyMap[][3] =
{
	// U+0300 COMBINING GRAVE ACCENT
	{L'̀', L'̀', L'`'},
	{L'̀', L'A', L'À'},
	{L'̀', L'E', L'È'},
	{L'̀', L'I', L'Ì'},
	{L'̀', L'O', L'Ò'},
	{L'̀', L'U', L'Ù'},
	{L'̀', L'a', L'à'},
	{L'̀', L'e', L'è'},
	{L'̀', L'i', L'ì'},
	{L'̀', L'o', L'ò'},
	{L'̀', L'u', L'ù'},

	// U+0301 COMBINING ACUTE ACCENT
	{L'́', L'́', L'´'},
	{L'́', L'A', L'Á'},
	{L'́', L'E', L'É'},
	{L'́', L'I', L'Í'},
	{L'́', L'O', L'Ó'},
	{L'́', L'U', L'Ú'},
	{L'́', L'Y', L'Ý'},
	{L'́', L'a', L'á'},
	{L'́', L'e', L'é'},
	{L'́', L'i', L'í'},
	{L'́', L'o', L'ó'},
	{L'́', L'u', L'ú'},
	{L'́', L'y', L'ý'},

	// U+0302 COMBINING CIRCUMFLEX ACCENT
	// = hat
	{L'̂', L'̂', L'^'},
	{L'̂', L'A', L'Â'},
	{L'̂', L'E', L'Ê'},
	{L'̂', L'I', L'Î'},
	{L'̂', L'O', L'Ô'},
	{L'̂', L'U', L'Û'},
	{L'̂', L'a', L'â'},
	{L'̂', L'e', L'ê'},
	{L'̂', L'i', L'î'},
	{L'̂', L'o', L'ô'},
	{L'̂', L'u', L'û'},

	// U+0303 COMBINING TILDE
	{L'̃', L'̃', L'~'},
	{L'̃', L'A', L'Ã'},
	{L'̃', L'N', L'Ñ'},
	{L'̃', L'O', L'Õ'},
	{L'̃', L'a', L'ã'},
	{L'̃', L'n', L'ñ'},
	{L'̃', L'o', L'õ'},

	// U+0308 COMBINING DIAERESIS
	{L'̈', L'̈', L'¨'},
	{L'̈', L'A', L'Ä'},
	{L'̈', L'E', L'Ë'},
	{L'̈', L'I', L'Ï'},
	{L'̈', L'O', L'Ö'},
	{L'̈', L'U', L'Ü'},
	{L'̈', L'Y', L'Ÿ'},
	{L'̈', L'a', L'ä'},
	{L'̈', L'e', L'ë'},
	{L'̈', L'i', L'ï'},
	{L'̈', L'o', L'ö'},
	{L'̈', L'u', L'ü'},
	{L'̈', L'y', L'ÿ'},

	// U+0327 COMBINING CEDILLA
	{L'̧', L'̧', L'¸'},
	{L'̧', L'C', L'Ç'},
	{L'̧', L'c', L'ç'},

	// U+030D COMBINING VERTICAL LINE ABOVE
	// * this is not the Greek tonos
	{L'̍', L'̍', L'\''},
	{L'̍', L'A', L'Á'},
	{L'̍', L'E', L'É'},
	{L'̍', L'I', L'Í'},
	{L'̍', L'O', L'Ó'},
	{L'̍', L'U', L'Ú'},
	{L'̍', L'Y', L'Ý'},
	{L'̍', L'C', L'Ç'},
	{L'̍', L'a', L'á'},
	{L'̍', L'e', L'é'},
	{L'̍', L'i', L'í'},
	{L'̍', L'o', L'ó'},
	{L'̍', L'u', L'ú'},
	{L'̍', L'y', L'ý'},
	{L'̍', L'c', L'ç'},

	// U+030E COMBINING DOUBLE VERTICAL LINE ABOVE
	{L'̎', L'̎', L'"'},
	{L'̎', L'A', L'Ä'},
	{L'̎', L'E', L'Ë'},
	{L'̎', L'I', L'Ï'},
	{L'̎', L'O', L'Ö'},
	{L'̎', L'U', L'Ü'},
	{L'̎', L'Y', L'Ÿ'},
	{L'̎', L'a', L'ä'},
	{L'̎', L'e', L'ë'},
	{L'̎', L'i', L'ï'},
	{L'̎', L'o', L'ö'},
	{L'̎', L'u', L'ü'},
	{L'̎', L'y', L'ÿ'},

	// U+0344 COMBINING GREEK DIALYTIKA TONOS
	// * use of this character is discouraged
	// muff1n: but not formally deprecated, sooo
	{L'̈́', L'̈́', L'΅'},
	{L'̈́', L'ι', L'ΐ'},
	{L'̈́', L'υ', L'ΰ'},

	// U+0308 COMBINING DIAERESIS
	// = Greek dialytika
	{L'̈', L'Ι', L'Ϊ'},
	{L'̈', L'Υ', L'Ϋ'},
	{L'̈', L'ι', L'ϊ'},
	{L'̈', L'υ', L'ϋ'},

	// U+0301 COMBINING ACUTE ACCENT
	// = Greek oxia, tonos
	{L'́', L'Α', L'Ά'},
	{L'́', L'Ε', L'Έ'},
	{L'́', L'Η', L'Ή'},
	{L'́', L'Ι', L'Ί'},
	{L'́', L'Ο', L'Ό'},
	{L'́', L'Υ', L'Ύ'},
	{L'́', L'Ω', L'Ώ'},
	{L'́', L'α', L'ά'},
	{L'́', L'ε', L'έ'},
	{L'́', L'η', L'ή'},
	{L'́', L'ι', L'ί'},
	{L'́', L'ο', L'ό'},
	{L'́', L'υ', L'ύ'},
	{L'́', L'ω', L'ώ'},
};

static struct RmJpRule const kprRmJpRule[] =
{
/* NOTE: Compiler bug: MWCC Wii 1.0 does not conform to C99 6.7.8p15. This
 * small macro delegation system is a workaround.
 */
#define MRJR_COUNT(_1, _2, _3, ...)	_3
#define MRJR_1(rm_, jp0_)			\
	{ARRAY_LENGTH(rm_) - 1, {rm_}, 1, {jp0_}}
#define MRJR_2(rm_, jp0_, jp1_)		\
	{ARRAY_LENGTH(rm_) - 1, {rm_}, 2, {jp0_, jp1_}}

#define MAKE_RM_JP_RULE(rm_, ...)	\
	MRJR_COUNT(__VA_ARGS__, MRJR_2, MRJR_1, _)(rm_, __VA_ARGS__)

	// 4
	MAKE_RM_JP_RULE("ltsa", L'っ', L'ぁ'),
	MAKE_RM_JP_RULE("ltse", L'っ', L'ぇ'),
	MAKE_RM_JP_RULE("ltsi", L'っ', L'ぃ'),
	MAKE_RM_JP_RULE("ltso", L'っ', L'ぉ'),
	MAKE_RM_JP_RULE("ltsu", L'っ'),
	MAKE_RM_JP_RULE("lwha", L'ぅ', L'ぁ'),
	MAKE_RM_JP_RULE("lwhe", L'ぅ', L'ぇ'),
	MAKE_RM_JP_RULE("lwhi", L'ぅ', L'ぃ'),
	MAKE_RM_JP_RULE("lwho", L'ぅ', L'ぉ'),
	MAKE_RM_JP_RULE("lwhu", L'ぅ'),
	MAKE_RM_JP_RULE("xtsa", L'っ', L'ぁ'),
	MAKE_RM_JP_RULE("xtse", L'っ', L'ぇ'),
	MAKE_RM_JP_RULE("xtsi", L'っ', L'ぃ'),
	MAKE_RM_JP_RULE("xtso", L'っ', L'ぉ'),
	MAKE_RM_JP_RULE("xtsu", L'っ'),
	MAKE_RM_JP_RULE("xwha", L'ぅ', L'ぁ'),
	MAKE_RM_JP_RULE("xwhe", L'ぅ', L'ぇ'),
	MAKE_RM_JP_RULE("xwhi", L'ぅ', L'ぃ'),
	MAKE_RM_JP_RULE("xwho", L'ぅ', L'ぉ'),
	MAKE_RM_JP_RULE("xwhu", L'ぅ'),

	// 3
	MAKE_RM_JP_RULE("bya", L'び', L'ゃ'),
	MAKE_RM_JP_RULE("bye", L'び', L'ぇ'),
	MAKE_RM_JP_RULE("byi", L'び', L'ぃ'),
	MAKE_RM_JP_RULE("byo", L'び', L'ょ'),
	MAKE_RM_JP_RULE("byu", L'び', L'ゅ'),
	MAKE_RM_JP_RULE("cha", L'ち', L'ゃ'),
	MAKE_RM_JP_RULE("che", L'ち', L'ぇ'),
	MAKE_RM_JP_RULE("chi", L'ち'),
	MAKE_RM_JP_RULE("cho", L'ち', L'ょ'),
	MAKE_RM_JP_RULE("chu", L'ち', L'ゅ'),
	MAKE_RM_JP_RULE("cya", L'ち', L'ゃ'),
	MAKE_RM_JP_RULE("cye", L'ち', L'ぇ'),
	MAKE_RM_JP_RULE("cyi", L'ち', L'ぃ'),
	MAKE_RM_JP_RULE("cyo", L'ち', L'ょ'),
	MAKE_RM_JP_RULE("cyu", L'ち', L'ゅ'),
	MAKE_RM_JP_RULE("dha", L'で', L'ゃ'),
	MAKE_RM_JP_RULE("dhe", L'で', L'ぇ'),
	MAKE_RM_JP_RULE("dhi", L'で', L'ぃ'),
	MAKE_RM_JP_RULE("dho", L'で', L'ょ'),
	MAKE_RM_JP_RULE("dhu", L'で', L'ゅ'),
	MAKE_RM_JP_RULE("dya", L'ぢ', L'ゃ'),
	MAKE_RM_JP_RULE("dyi", L'ぢ', L'ぃ'),
	MAKE_RM_JP_RULE("dye", L'ぢ', L'ぇ'),
	MAKE_RM_JP_RULE("dyo", L'ぢ', L'ょ'),
	MAKE_RM_JP_RULE("dyu", L'ぢ', L'ゅ'),
	MAKE_RM_JP_RULE("fya", L'ふ', L'ゃ'),
	MAKE_RM_JP_RULE("fye", L'ふ', L'ぇ'),
	MAKE_RM_JP_RULE("fyi", L'ふ', L'ぃ'),
	MAKE_RM_JP_RULE("fyo", L'ふ', L'ょ'),
	MAKE_RM_JP_RULE("fyu", L'ふ', L'ゅ'),
	MAKE_RM_JP_RULE("gya", L'ぎ', L'ゃ'),
	MAKE_RM_JP_RULE("gye", L'ぎ', L'ぇ'),
	MAKE_RM_JP_RULE("gyi", L'ぎ', L'ぃ'),
	MAKE_RM_JP_RULE("gyo", L'ぎ', L'ょ'),
	MAKE_RM_JP_RULE("gyu", L'ぎ', L'ゅ'),
	MAKE_RM_JP_RULE("hya", L'ひ', L'ゃ'),
	MAKE_RM_JP_RULE("hye", L'ひ', L'ぇ'),
	MAKE_RM_JP_RULE("hyi", L'ひ', L'ぃ'),
	MAKE_RM_JP_RULE("hyo", L'ひ', L'ょ'),
	MAKE_RM_JP_RULE("hyu", L'ひ', L'ゅ'),
	MAKE_RM_JP_RULE("jya", L'じ', L'ゃ'),
	MAKE_RM_JP_RULE("jye", L'じ', L'ぇ'),
	MAKE_RM_JP_RULE("jyi", L'じ', L'ぃ'),
	MAKE_RM_JP_RULE("jyo", L'じ', L'ょ'),
	MAKE_RM_JP_RULE("jyu", L'じ', L'ゅ'),
	MAKE_RM_JP_RULE("kya", L'き', L'ゃ'),
	MAKE_RM_JP_RULE("kye", L'き', L'ぇ'),
	MAKE_RM_JP_RULE("kyi", L'き', L'ぃ'),
	MAKE_RM_JP_RULE("kyo", L'き', L'ょ'),
	MAKE_RM_JP_RULE("kyu", L'き', L'ゅ'),
	MAKE_RM_JP_RULE("lya", L'ゃ'),
	MAKE_RM_JP_RULE("lyo", L'ょ'),
	MAKE_RM_JP_RULE("lyu", L'ゅ'),
	MAKE_RM_JP_RULE("ltu", L'っ'),
	MAKE_RM_JP_RULE("lwa", L'ゎ'),
	MAKE_RM_JP_RULE("lwe", L'ぅ', L'ぇ'),
	MAKE_RM_JP_RULE("lwi", L'ぅ', L'ぃ'),
	MAKE_RM_JP_RULE("lwu", L'ぅ'),
	MAKE_RM_JP_RULE("mya", L'み', L'ゃ'),
	MAKE_RM_JP_RULE("mye", L'み', L'ぇ'),
	MAKE_RM_JP_RULE("myi", L'み', L'ぃ'),
	MAKE_RM_JP_RULE("myo", L'み', L'ょ'),
	MAKE_RM_JP_RULE("myu", L'み', L'ゅ'),
	MAKE_RM_JP_RULE("nya", L'に', L'ゃ'),
	MAKE_RM_JP_RULE("nye", L'に', L'ぇ'),
	MAKE_RM_JP_RULE("nyi", L'に', L'ぃ'),
	MAKE_RM_JP_RULE("nyo", L'に', L'ょ'),
	MAKE_RM_JP_RULE("nyu", L'に', L'ゅ'),
	MAKE_RM_JP_RULE("pya", L'ぴ', L'ゃ'),
	MAKE_RM_JP_RULE("pye", L'ぴ', L'ぇ'),
	MAKE_RM_JP_RULE("pyi", L'ぴ', L'ぃ'),
	MAKE_RM_JP_RULE("pyo", L'ぴ', L'ょ'),
	MAKE_RM_JP_RULE("pyu", L'ぴ', L'ゅ'),
	MAKE_RM_JP_RULE("qya", L'く', L'ゃ'),
	MAKE_RM_JP_RULE("qye", L'く', L'ぇ'),
	MAKE_RM_JP_RULE("qyi", L'く', L'ぃ'),
	MAKE_RM_JP_RULE("qyo", L'く', L'ょ'),
	MAKE_RM_JP_RULE("qyu", L'く', L'ゅ'),
	MAKE_RM_JP_RULE("rya", L'り', L'ゃ'),
	MAKE_RM_JP_RULE("rye", L'り', L'ぇ'),
	MAKE_RM_JP_RULE("ryi", L'り', L'ぃ'),
	MAKE_RM_JP_RULE("ryo", L'り', L'ょ'),
	MAKE_RM_JP_RULE("ryu", L'り', L'ゅ'),
	MAKE_RM_JP_RULE("sha", L'し', L'ゃ'),
	MAKE_RM_JP_RULE("she", L'し', L'ぇ'),
	MAKE_RM_JP_RULE("shi", L'し'),
	MAKE_RM_JP_RULE("sho", L'し', L'ょ'),
	MAKE_RM_JP_RULE("shu", L'し', L'ゅ'),
	MAKE_RM_JP_RULE("sya", L'し', L'ゃ'),
	MAKE_RM_JP_RULE("sye", L'し', L'ぇ'),
	MAKE_RM_JP_RULE("syi", L'し', L'ぃ'),
	MAKE_RM_JP_RULE("syo", L'し', L'ょ'),
	MAKE_RM_JP_RULE("syu", L'し', L'ゅ'),
	MAKE_RM_JP_RULE("tha", L'て', L'ゃ'),
	MAKE_RM_JP_RULE("the", L'て', L'ぇ'),
	MAKE_RM_JP_RULE("thi", L'て', L'ぃ'),
	MAKE_RM_JP_RULE("tho", L'て', L'ょ'),
	MAKE_RM_JP_RULE("thu", L'て', L'ゅ'),
	MAKE_RM_JP_RULE("tsa", L'つ', L'ぁ'),
	MAKE_RM_JP_RULE("tse", L'つ', L'ぇ'),
	MAKE_RM_JP_RULE("tsi", L'つ', L'ぃ'),
	MAKE_RM_JP_RULE("tso", L'つ', L'ぉ'),
	MAKE_RM_JP_RULE("tsu", L'つ'),
	MAKE_RM_JP_RULE("tya", L'ち', L'ゃ'),
	MAKE_RM_JP_RULE("tye", L'ち', L'ぇ'),
	MAKE_RM_JP_RULE("tyi", L'ち', L'ぃ'),
	MAKE_RM_JP_RULE("tyo", L'ち', L'ょ'),
	MAKE_RM_JP_RULE("tyu", L'ち', L'ゅ'),
	MAKE_RM_JP_RULE("vya", L'ヴ', L'ゃ'),
	MAKE_RM_JP_RULE("vye", L'ヴ', L'ぇ'),
	MAKE_RM_JP_RULE("vyi", L'ヴ', L'ぃ'),
	MAKE_RM_JP_RULE("vyo", L'ヴ', L'ょ'),
	MAKE_RM_JP_RULE("vyu", L'ヴ', L'ゅ'),
	MAKE_RM_JP_RULE("wha", L'う', L'ぁ'),
	MAKE_RM_JP_RULE("whe", L'う', L'ぇ'),
	MAKE_RM_JP_RULE("whi", L'う', L'ぃ'),
	MAKE_RM_JP_RULE("who", L'う', L'ぉ'),
	MAKE_RM_JP_RULE("whu", L'う'),
	MAKE_RM_JP_RULE("xya", L'ゃ'),
	MAKE_RM_JP_RULE("xyo", L'ょ'),
	MAKE_RM_JP_RULE("xyu", L'ゅ'),
	MAKE_RM_JP_RULE("xtu", L'っ'),
	MAKE_RM_JP_RULE("xwa", L'ゎ'),
	MAKE_RM_JP_RULE("xwe", L'ぅ', L'ぇ'),
	MAKE_RM_JP_RULE("xwi", L'ぅ', L'ぃ'),
	MAKE_RM_JP_RULE("xwu", L'ぅ'),
	MAKE_RM_JP_RULE("zya", L'じ', L'ゃ'),
	MAKE_RM_JP_RULE("zye", L'じ', L'ぇ'),
	MAKE_RM_JP_RULE("zyi", L'じ', L'ぃ'),
	MAKE_RM_JP_RULE("zyo", L'じ', L'ょ'),
	MAKE_RM_JP_RULE("zyu", L'じ', L'ゅ'),

	// 2
	MAKE_RM_JP_RULE("ba", L'ば'),
	MAKE_RM_JP_RULE("be", L'べ'),
	MAKE_RM_JP_RULE("bi", L'び'),
	MAKE_RM_JP_RULE("bo", L'ぼ'),
	MAKE_RM_JP_RULE("bu", L'ぶ'),
	MAKE_RM_JP_RULE("ca", L'か'),
	MAKE_RM_JP_RULE("ce", L'せ'),
	MAKE_RM_JP_RULE("ci", L'し'),
	MAKE_RM_JP_RULE("co", L'こ'),
	MAKE_RM_JP_RULE("cu", L'く'),
	MAKE_RM_JP_RULE("da", L'だ'),
	MAKE_RM_JP_RULE("de", L'で'),
	MAKE_RM_JP_RULE("di", L'ぢ'),
	MAKE_RM_JP_RULE("do", L'ど'),
	MAKE_RM_JP_RULE("du", L'づ'),
	MAKE_RM_JP_RULE("fa", L'ふ', L'ぁ'),
	MAKE_RM_JP_RULE("fe", L'ふ', L'ぇ'),
	MAKE_RM_JP_RULE("fi", L'ふ', L'ぃ'),
	MAKE_RM_JP_RULE("fo", L'ふ', L'ぉ'),
	MAKE_RM_JP_RULE("fu", L'ふ'),
	MAKE_RM_JP_RULE("ga", L'が'),
	MAKE_RM_JP_RULE("ge", L'げ'),
	MAKE_RM_JP_RULE("gi", L'ぎ'),
	MAKE_RM_JP_RULE("go", L'ご'),
	MAKE_RM_JP_RULE("gu", L'ぐ'),
	MAKE_RM_JP_RULE("ha", L'は'),
	MAKE_RM_JP_RULE("he", L'へ'),
	MAKE_RM_JP_RULE("hi", L'ひ'),
	MAKE_RM_JP_RULE("ho", L'ほ'),
	MAKE_RM_JP_RULE("hu", L'ふ'),
	MAKE_RM_JP_RULE("ja", L'じ', L'ゃ'),
	MAKE_RM_JP_RULE("je", L'じ', L'ぇ'),
	MAKE_RM_JP_RULE("ji", L'じ'),
	MAKE_RM_JP_RULE("jo", L'じ', L'ょ'),
	MAKE_RM_JP_RULE("ju", L'じ', L'ゅ'),
	MAKE_RM_JP_RULE("ka", L'か'),
	MAKE_RM_JP_RULE("ke", L'け'),
	MAKE_RM_JP_RULE("ki", L'き'),
	MAKE_RM_JP_RULE("ko", L'こ'),
	MAKE_RM_JP_RULE("ku", L'く'),
	MAKE_RM_JP_RULE("la", L'ぁ'),
	MAKE_RM_JP_RULE("le", L'ぇ'),
	MAKE_RM_JP_RULE("li", L'ぃ'),
	MAKE_RM_JP_RULE("lo", L'ぉ'),
	MAKE_RM_JP_RULE("lu", L'ぅ'),
	MAKE_RM_JP_RULE("ma", L'ま'),
	MAKE_RM_JP_RULE("me", L'め'),
	MAKE_RM_JP_RULE("mi", L'み'),
	MAKE_RM_JP_RULE("mo", L'も'),
	MAKE_RM_JP_RULE("mu", L'む'),
	MAKE_RM_JP_RULE("na", L'な'),
	MAKE_RM_JP_RULE("ne", L'ね'),
	MAKE_RM_JP_RULE("ni", L'に'),
	MAKE_RM_JP_RULE("no", L'の'),
	MAKE_RM_JP_RULE("nu", L'ぬ'),
	MAKE_RM_JP_RULE("pa", L'ぱ'),
	MAKE_RM_JP_RULE("pe", L'ぺ'),
	MAKE_RM_JP_RULE("pi", L'ぴ'),
	MAKE_RM_JP_RULE("po", L'ぽ'),
	MAKE_RM_JP_RULE("pu", L'ぷ'),
	MAKE_RM_JP_RULE("qa", L'く', L'ぁ'),
	MAKE_RM_JP_RULE("qe", L'く', L'ぇ'),
	MAKE_RM_JP_RULE("qi", L'く', L'ぃ'),
	MAKE_RM_JP_RULE("qo", L'く', L'ぉ'),
	MAKE_RM_JP_RULE("qu", L'く'),
	MAKE_RM_JP_RULE("ra", L'ら'),
	MAKE_RM_JP_RULE("re", L'れ'),
	MAKE_RM_JP_RULE("ri", L'り'),
	MAKE_RM_JP_RULE("ro", L'ろ'),
	MAKE_RM_JP_RULE("ru", L'る'),
	MAKE_RM_JP_RULE("sa", L'さ'),
	MAKE_RM_JP_RULE("se", L'せ'),
	MAKE_RM_JP_RULE("si", L'し'),
	MAKE_RM_JP_RULE("so", L'そ'),
	MAKE_RM_JP_RULE("su", L'す'),
	MAKE_RM_JP_RULE("ta", L'た'),
	MAKE_RM_JP_RULE("te", L'て'),
	MAKE_RM_JP_RULE("ti", L'ち'),
	MAKE_RM_JP_RULE("to", L'と'),
	MAKE_RM_JP_RULE("tu", L'つ'),
	MAKE_RM_JP_RULE("va", L'ヴ', L'ぁ'),
	MAKE_RM_JP_RULE("ve", L'ヴ', L'ぇ'),
	MAKE_RM_JP_RULE("vi", L'ヴ', L'ぃ'),
	MAKE_RM_JP_RULE("vo", L'ヴ', L'ぉ'),
	MAKE_RM_JP_RULE("vu", L'ヴ'),
	MAKE_RM_JP_RULE("wa", L'わ'),
	MAKE_RM_JP_RULE("we", L'う', L'ぇ'),
	MAKE_RM_JP_RULE("wi", L'う', L'ぃ'),
	MAKE_RM_JP_RULE("wo", L'を'),
	MAKE_RM_JP_RULE("wu", L'う'),
	MAKE_RM_JP_RULE("xa", L'ぁ'),
	MAKE_RM_JP_RULE("xe", L'ぇ'),
	MAKE_RM_JP_RULE("xi", L'ぃ'),
	MAKE_RM_JP_RULE("xo", L'ぉ'),
	MAKE_RM_JP_RULE("xu", L'ぅ'),
	MAKE_RM_JP_RULE("ya", L'や'),
	MAKE_RM_JP_RULE("ye", L'い', L'ぇ'),
	MAKE_RM_JP_RULE("yi", L'い'),
	MAKE_RM_JP_RULE("yo", L'よ'),
	MAKE_RM_JP_RULE("yu", L'ゆ'),
	MAKE_RM_JP_RULE("za", L'ざ'),
	MAKE_RM_JP_RULE("ze", L'ぜ'),
	MAKE_RM_JP_RULE("zi", L'じ'),
	MAKE_RM_JP_RULE("zo", L'ぞ'),
	MAKE_RM_JP_RULE("zu", L'ず'),

	MAKE_RM_JP_RULE("nn", L'ん'),

	// 1
	MAKE_RM_JP_RULE("a", L'あ'),
	MAKE_RM_JP_RULE("e", L'え'),
	MAKE_RM_JP_RULE("i", L'い'),
	MAKE_RM_JP_RULE("o", L'お'),
	MAKE_RM_JP_RULE("u", L'う'),

#undef MAKE_RM_JP_RULE
#undef MRJR_1
#undef MRJR_2

#undef MRJR_COUNT
};

static wchar_t const kprLookupTable1252[] =
{
	L'€' , L'\0', L'‚' , L'ƒ' , L'„' , L'…' , L'†' , L'‡' ,
	L'ˆ' , L'‰' , L'Š' , L'‹' , L'Œ' , L'\0', L'Ž' , L'\0',
	L'\0', L'‘' , L'’' , L'“' , L'”' , L'•' , L'–' , L'—' ,
	L'˜' , L'™' , L'š' , L'›' , L'œ' , L'\0', L'ž' , L'Ÿ' ,
};

static wchar_t const kprLookupTable437[] =
{
	L'Ç', L'ü', L'é', L'â', L'ä', L'à', L'å', L'ç',
	L'ê', L'ë', L'è', L'ï', L'î', L'ì', L'Ä', L'Å',
	L'É', L'æ', L'Æ', L'ô', L'ö', L'ò', L'û', L'ù',
	L'ÿ', L'Ö', L'Ü', L'¢', L'£', L'¥', L'₧', L'ƒ',
	L'á', L'í', L'ó', L'ú', L'ñ', L'Ñ', L'ª', L'º',
	L'¿', L'⌐', L'¬', L'½', L'¼', L'¡', L'«', L'»',
	L'░', L'▒', L'▓', L'│', L'┤', L'╡', L'╢', L'╖',
	L'╕', L'╣', L'║', L'╗', L'╝', L'╜', L'╛', L'┐',
	L'└', L'┴', L'┬', L'├', L'─', L'┼', L'╞', L'╟',
	L'╚', L'╔', L'╩', L'╦', L'╠', L'═', L'╬', L'╧',
	L'╨', L'╤', L'╥', L'╙', L'╘', L'╒', L'╓', L'╫',
	L'╪', L'┘', L'┌', L'█', L'▄', L'▌', L'▐', L'▀',
	L'α', L'ß', L'Γ', L'π', L'Σ', L'σ', L'µ', L'τ',
	L'Φ', L'Θ', L'Ω', L'δ', L'∞', L'φ', L'ε', L'∩',
	L'≡', L'±', L'≥', L'≤', L'⌠', L'⌡', L'÷', L'≈',
	L'°', L'∙', L'·', L'√', L'ⁿ', L'²', L'■', L'\u00a0', // <NBSP> (nonprinting)
};

// .data, .sdata
char const *__KPRVersion = RVL_SDK_LIB_VERSION_STRING(KPR);

// .sdata
static KPRProcessDeadKeysFunc *kprProcDeadKeysFP = NULL;
static KPRProcessRomajiFunc *kprProcRomajiFP = NULL;

// .sdata2
static wchar_t const kprConvertSmallTsu[2] = {L'っ', L'ッ'};
static wchar_t const kprConvertNN[2] = {L'ん', L'ン'};

/*******************************************************************************
 * functions
 */

void KPRInitRegionUS(void)
{
	kprProcDeadKeysFP = &KPRProcessDeadKeys;
}

void KPRInitRegionJP(void)
{
	kprProcRomajiFP = &KPRProcessRomaji;
}

void KPRInitRegionEU(void)
{
	kprProcDeadKeysFP = &KPRProcessDeadKeys;
}

void KPRInitQueue(KPRQueue *queue)
{
	static u8 once;

	OSAssert_Line(111, queue);

	if (!once)
	{
		OSRegisterVersion(__KPRVersion);
		once = TRUE;
	}

	KPRSetMode(queue, KPR_MODE_ALT_KEYPAD);
}

void KPRClearQueue(KPRQueue *queue)
{
	OSAssert_Line(133, queue);

	queue->oCount = 0;
	queue->iCount = 0;
	queue->altVal = 0;
}

void KPRSetMode(KPRQueue *queue, KPRMode mode)
{
	OSAssert_Line(153, queue);
	OSAssert_Line(156,(mode & (KPR_MODE_DEADKEY | KPR_MODE_JP_ROMAJI_HIRAGANA))
		!= (KPR_MODE_DEADKEY | KPR_MODE_JP_ROMAJI_HIRAGANA));
	OSAssert_Line(158, (mode & (KPR_MODE_DEADKEY | KPR_MODE_JP_ROMAJI_KATAKANA))
		!= (KPR_MODE_DEADKEY | KPR_MODE_JP_ROMAJI_KATAKANA));
	OSAssert_Line(160, (mode & (KPR_MODE_JP_ROMAJI_HIRAGANA
		| KPR_MODE_JP_ROMAJI_KATAKANA)) != (KPR_MODE_JP_ROMAJI_HIRAGANA
		| KPR_MODE_JP_ROMAJI_KATAKANA));

	OSAssertMessage_Line(
		163, !(mode & KPR_MODE_DEADKEY) || kprProcDeadKeysFP != NULL,
		"KPRSetMode: KPR_MODE_DEADKEY selected, but US/EU KPR region not set.");
	OSAssertMessage_Line(165,
	                     !(mode & KPR_MODE_JP_ROMAJI_HIRAGANA)
	                         || kprProcRomajiFP != NULL,
	                     "KPRSetMode: KPR_MODE_JP_ROMAJI_HIRAGANA selected, "
	                     "but JP region not set.");
	OSAssertMessage_Line(167,
	                     !(mode & KPR_MODE_JP_ROMAJI_KATAKANA)
	                         || kprProcRomajiFP != NULL,
	                     "KPRSetMode: KPR_MODE_JP_ROMAJI_KATAKANA selected, "
	                     "but JP region not set.");

	queue->mode = mode;

	KPRClearQueue(queue);
}

KPRMode KPRGetMode(KPRQueue *queue)
{
	OSAssert_Line(185, queue);

	return queue->mode;
}

u32 KPRPutChar(KPRQueue *queue, wchar_t ch)
{
	OSAssert_Line(205, queue);
	OSAssertMessage_Line(208, !queue->oCount,
	                     "KPRPutChar: queue was not emptied");

	if (queue->oCount + queue->iCount + 1
	    >= (int)ARRAY_LENGTH(queue->text) - 1)
	{
		OSError_Line(215, "KPRPutChar: Overflow");
	}

	BOOL intrStatus = OSDisableInterrupts();

	if (!(queue->mode & KPR_MODE_ALT_KEYPAD)
		|| !KPRProcessAltKeypad(queue, ch))
	{
		queue->text[queue->oCount + queue->iCount] = ch;

		++queue->iCount;

		if (queue->mode & KPR_MODE_DEADKEY)
		{
			OSAssert_Line(231, kprProcDeadKeysFP != NULL);

			(*kprProcDeadKeysFP)(queue);
		}
		else if (queue->mode & KPR_MODE_JP_ROMAJI_KATAKANA
		         || queue->mode & KPR_MODE_JP_ROMAJI_HIRAGANA)
		{
			OSAssert_Line(235, kprProcRomajiFP != NULL);

			(*kprProcRomajiFP)(queue);
		}
		else
		{
			queue->oCount = queue->iCount;
			queue->iCount = 0;
		}

		if ((wint_t)ch == WEOF)
			--queue->oCount;
	}

	OSRestoreInterrupts(intrStatus);

	return queue->oCount;
}

wchar_t KPRGetChar(KPRQueue *queue)
{
	OSAssert_Line(270, queue);

	if (queue->oCount == 0)
		return L'\0';

	BOOL intrStatus = OSDisableInterrupts();
	wchar_t ch = queue->text[0];

	u32 i;
	for (i = 1; i < queue->iCount + queue->oCount; ++i)
		queue->text[i - 1] = queue->text[i];

	--queue->oCount;

	OSRestoreInterrupts(intrStatus);

	return ch;
}

wchar_t KPRRemoveChar(KPRQueue *queue)
{
	wchar_t ch = L'\0';

	OSAssert_Line(306, queue);

	BOOL intrStatus = OSDisableInterrupts();

	if (queue->iCount)
	{
		ch = queue->text[queue->oCount + queue->iCount - 1];

		--queue->iCount;
	}

	OSRestoreInterrupts(intrStatus);

	return ch;
}

u8 KPRLookAhead(KPRQueue *queue, wchar_t *buffer, u32 count)
{
	OSAssert_Line(344, queue);

	if (buffer == NULL || !count)
		return queue->iCount + queue->oCount;

	BOOL intrStatus = OSDisableInterrupts();

	u8 i;
	for (i = 0; i < queue->iCount + queue->oCount && i < count; ++i)
		buffer[i] = queue->text[i];

	if (i < count)
		buffer[i] = L'\0';

	OSRestoreInterrupts(intrStatus);

	return queue->iCount + queue->oCount;
}

static BOOL KPRProcessAltKeypad(KPRQueue *queue, wchar_t ch)
{
	int i;
	u32 b;
	u32 a;

	OSAssert_Line(385, queue);

	if (queue->altVal)
	{
		a = queue->altVal & 0x80000000;
		queue->altVal &= ~0x80000000;

		if (ch >= 0xf130 && ch <= 0xf139)
		{
			if (queue->altVal > 0x006666666)
				return TRUE;

			queue->altVal = a | queue->altVal * 10 + (ch - 0xf130);
			return TRUE;
		}

		b = queue->altVal;
		if (b >= 0x80 && b <= 0xff)
		{
			if (a)
			{
				if (b < 0xa0)
					b = kprLookupTable1252[b - 0x80];
			}
			else
			{
				b = kprLookupTable437[b - 0x80];
			}
		}
		else if (b > 0xff)
		{
			if (b > 0x06666666)
				b = 0x20;
			else
				b &= 0xff;
		}

		for (i = queue->oCount + queue->iCount; i > queue->oCount; --i)
			queue->text[i] = queue->text[i - 1];

		queue->text[queue->oCount] = b;
		++queue->oCount;

		queue->altVal = 0;

		if (ch == L'\0')
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		if (ch >= 0xf130 && ch <= 0xf139)
		{
			if (ch == 0xf130)
				queue->altVal = 0x80000000;
			else
				queue->altVal = ch - 0xf130;

			return TRUE;
		}

		if (ch == L'\0')
			return TRUE;
		else
			return FALSE;
	}
}

static void KPRProcessDeadKeys(KPRQueue *queue)
{
	u32 i;
	u32 first;
	u32 second;

	OSAssert_Line(479, queue);
	OSAssert_Line(480, queue->iCount == 1 || queue->iCount == 2);

	if (queue->iCount == 1)
	{
		for (i = 0; i < ARRAY_LENGTH(kprDeadKeyMap); ++i)
		{
			if (queue->text[queue->oCount] == kprDeadKeyMap[i][0])
				return;
		}

		++queue->oCount;
		queue->iCount = 0;
	}
	else
	{
		second = ARRAY_LENGTH(kprDeadKeyMap);
		for (i = 0; i < ARRAY_LENGTH(kprDeadKeyMap); ++i)
		{
			if (queue->text[queue->oCount] == kprDeadKeyMap[i][0]
			    && queue->text[queue->oCount + 1] == kprDeadKeyMap[i][1])
			{
				queue->text[queue->oCount] = kprDeadKeyMap[i][2];
				++queue->oCount;

				queue->iCount = 0;

				return;
			}

			if (queue->text[queue->oCount] == kprDeadKeyMap[i][0]
			    && queue->text[queue->oCount] == kprDeadKeyMap[i][1])
			{
				first = i;
			}

			if (queue->text[queue->oCount + 1] == kprDeadKeyMap[i][0]
			    && queue->text[queue->oCount + 1] == kprDeadKeyMap[i][1])
			{
				second = i;
			}
		}

		queue->text[queue->oCount] = kprDeadKeyMap[first][2];

		if (second < ARRAY_LENGTH(kprDeadKeyMap))
			queue->text[queue->oCount + 1] = kprDeadKeyMap[second][2];

		queue->oCount += 2;
		queue->iCount = 0;
	}
}

static void KPRProcessRomaji(KPRQueue *queue)
{
	OSAssert_Line(566, queue);
	OSAssert_Line(567, queue->iCount > 0 && queue->iCount < 5);

	u8 count = queue->oCount;
	u8 iCount = queue->iCount;
	wchar_t *buffer = queue->text;
	wchar_t last = buffer[count + iCount - 1];
	u8 isKatakana = (queue->mode & KPR_MODE_JP_ROMAJI_KATAKANA)
	             == KPR_MODE_JP_ROMAJI_KATAKANA;

	if (last < L'a' || last > L'z')
	{
		if (iCount > 1 && buffer[count + iCount - 2] == L'n')
			buffer[count + iCount - 2] = kprConvertNN[isKatakana];

		queue->oCount += iCount;
		queue->iCount = 0;

		return;
	}

	u32 i;
	u8 j;
	u8 rmLen;
	for (i = 0; i < ARRAY_LENGTH(kprRmJpRule); ++i)
	{
		rmLen = kprRmJpRule[i].rmLen;

		if (iCount < rmLen)
			continue;

		for (j = 0; j < rmLen; ++j)
		{
			if ((wint_t)buffer[count + iCount - 1 - j]
			    != (wint_t)kprRmJpRule[i].rm[rmLen - 1 - j])
			{
				break;
			}
		}

		if (j == rmLen)
			break;
	}

	if (i < ARRAY_LENGTH(kprRmJpRule))
	{
		if (iCount > rmLen)
		{
			if (iCount - rmLen == 2)
			{
				count = queue->oCount += 2;
				iCount = queue->iCount -= 2;
			}
			else if (buffer[count] == buffer[count + 1])
			{
				buffer[count] = kprConvertSmallTsu[isKatakana];
			}
			else
			{
				OSAssertMessage_Line(635, FALSE, "KPR: Unexpected condition");
			}
		}

		for (j = 0; j < kprRmJpRule[i].jpLen; ++j)
		{
			if (kprRmJpRule[i].jp[j] < L'ァ')
			{
				buffer[count + iCount - rmLen + j] =
					kprRmJpRule[i].jp[j] + isKatakana * 0x60;
			}
			else
			{
				buffer[count + iCount - rmLen + j] = kprRmJpRule[i].jp[j];
			}
		}

		queue->oCount += iCount - rmLen + j;
		queue->iCount = 0;

		return;
	}

	if (iCount == 2 && buffer[count] == L'n' && isConsonant(buffer[count + 1])
	    && buffer[count + 1] != L'y')
	{
		buffer[count] = kprConvertNN[isKatakana];

		++queue->oCount;
		--queue->iCount;

		return;
	}

	if (iCount == 4)
	{
		count = ++queue->oCount;
		iCount = --queue->iCount;
	}

	if (iCount == 3)
	{
		if ((buffer[count] == L'l' || buffer[count] == L'x')
		    && buffer[count + 1] == L't' && buffer[count + 2] == L's')
		{
			return;
		}

		if ((buffer[count] == L'l' || buffer[count] == L'x')
		    && buffer[count + 1] == L'w' && buffer[count + 2] == L'h')
		{
			return;
		}

		if (buffer[count] == buffer[count + 1]
		    && ((buffer[count + 1] != L'w' && buffer[count + 1] != L'y'
		         && buffer[count + 2] == L'y')
		        || (buffer[count + 1] == L't' && buffer[count + 2] == L's')
		        || (buffer[count + 2] == L'h'
		            && wcschr(L"csdtw", buffer[count + 1]))
		        || (buffer[count + 1] == L'l'
		            && (buffer[count + 2] == L't' || buffer[count + 2] == L'w'))
		        || (buffer[count + 1] == L'x'
		            && (buffer[count + 2] == L't'
		                || buffer[count + 2] == L'w'))))
		{
			return;
		}

		count = ++queue->oCount;
		iCount = --queue->iCount;
	}

	if (iCount == 2)
	{
		if (buffer[count] == buffer[count + 1]
		    || (buffer[count] != L'w' && buffer[count + 1] == L'y')
		    || (buffer[count] == L't' && buffer[count + 1] == L's')
		    || (buffer[count + 1] == L'h' && wcschr(L"csdtw", buffer[count]))
		    || (buffer[count] == L'l'
		        && (buffer[count + 1] == L't' || buffer[count + 1] == L'w'))
		    || (buffer[count] == L'x'
		        && (buffer[count + 1] == L't' || buffer[count + 1] == L'w')))
		{
			return;
		}

		++queue->oCount;
		--queue->iCount;
	}
}
