/*
* settings.c: read and write saved sessions. (platform-independent)
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "putty.h"
#include "storage.h"

/* The cipher order given here is the default order. */
static const struct keyvalwhere ciphernames[] = {
	{ "aes",        CIPHER_AES,             -1, -1 },
	{ "blowfish",   CIPHER_BLOWFISH,        -1, -1 },
	{ "3des",       CIPHER_3DES,            -1, -1 },
	{ "WARN",       CIPHER_WARN,            -1, -1 },
	{ "arcfour",    CIPHER_ARCFOUR,         -1, -1 },
	{ "des",        CIPHER_DES,             -1, -1 }
};

static const struct keyvalwhere kexnames[] = {
	{ "dh-gex-sha1",        KEX_DHGEX,      -1, -1 },
	{ "dh-group14-sha1",    KEX_DHGROUP14,  -1, -1 },
	{ "dh-group1-sha1",     KEX_DHGROUP1,   -1, -1 },
	{ "rsa",                KEX_RSA,        KEX_WARN, -1 },
	{ "WARN",               KEX_WARN,       -1, -1 }
};

/*
* All the terminal modes that we know about for the "TerminalModes"
* setting. (Also used by config.c for the drop-down list.)
* This is currently precisely the same as the set in ssh.c, but could
* in principle differ if other backends started to support tty modes
* (e.g., the pty backend).
*/
const char *const ttymodes[] = {
	"INTR",	"QUIT",     "ERASE",	"KILL",     "EOF",
	"EOL",	"EOL2",     "START",	"STOP",     "SUSP",
	"DSUSP",	"REPRINT",  "WERASE",	"LNEXT",    "FLUSH",
	"SWTCH",	"STATUS",   "DISCARD",	"IGNPAR",   "PARMRK",
	"INPCK",	"ISTRIP",   "INLCR",	"IGNCR",    "ICRNL",
	"IUCLC",	"IXON",     "IXANY",	"IXOFF",    "IMAXBEL",
	"ISIG",	"ICANON",   "XCASE",	"ECHO",     "ECHOE",
	"ECHOK",	"ECHONL",   "NOFLSH",	"TOSTOP",   "IEXTEN",
	"ECHOCTL",	"ECHOKE",   "PENDIN",	"OPOST",    "OLCUC",
	"ONLCR",	"OCRNL",    "ONOCR",	"ONLRET",   "CS7",
	"CS8",	"PARENB",   "PARODD",	NULL
};

  //----------------------------------------------------------------------------------------
  // get the code page from terminal encoding or font
  //----------------------------------------------------------------------------------------
  struct CpData {
      int cp;
      char * name;
  };
  static struct CpData CPDATA[] = { { 37, "IBM037" }, // IBM EBCDIC US-Canada
      { 437, "IBM437" }, // OEM United States
      { 500, "IBM500" }, // IBM EBCDIC International
      { 708, "ASMO-708" }, // Arabic (ASMO 708)
      { 709, "Arabic" }, // (ASMO-449+, BCON V4)
      { 710, "Arabic" }, // - Transparent Arabic
      { 720, "DOS-720" }, // Arabic (Transparent ASMO); Arabic (DOS)
      { 737, "ibm737" }, // OEM Greek (formerly 437G); Greek (DOS)
      { 775, "ibm775" }, // OEM Baltic; Baltic (DOS)
      { 850, "ibm850" }, // OEM Multilingual Latin 1; Western European (DOS)
      { 852, "ibm852" }, // OEM Latin 2; Central European (DOS)
      { 855, "IBM855" }, // OEM Cyrillic (primarily Russian)
      { 857, "ibm857" }, // OEM Turkish; Turkish (DOS)
      { 858, "IBM00858" }, // OEM Multilingual Latin 1 + Euro symbol
      { 860, "IBM860" }, // OEM Portuguese; Portuguese (DOS)
      { 861, "ibm861" }, // OEM Icelandic; Icelandic (DOS)
      { 862, "DOS-862" }, // OEM Hebrew; Hebrew (DOS)
      { 863, "IBM863" }, // OEM French Canadian; French Canadian (DOS)
      { 864, "IBM864" }, // OEM Arabic; Arabic (864)
      { 865, "IBM865" }, // OEM Nordic; Nordic (DOS)
      { 866, "cp866" }, // OEM Russian; Cyrillic (DOS)
      { 869, "ibm869" }, // OEM Modern Greek; Greek, Modern (DOS)
      { 870, "IBM870" }, // IBM EBCDIC Multilingual/ROECE (Latin 2); IBM EBCDIC Multilingual Latin 2
      { 874, "windows-874" }, // ANSI/OEM Thai (same as 28605, ISO 8859-15); Thai (Windows)
      { 875, "cp875" }, // IBM EBCDIC Greek Modern
      { 932, "shift_jis" }, // ANSI/OEM Japanese; Japanese (Shift-JIS)
      { 936, "gb2312" }, // ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312)
      { 949, "ks_c_5601-1987" }, // ANSI/OEM Korean (Unified Hangul Code)
      { 950, "big5" }, // ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5)
      { 1026, "IBM1026" }, // IBM EBCDIC Turkish (Latin 5)
      { 1047, "IBM01047" }, // IBM EBCDIC Latin 1/Open System
      { 1140, "IBM01140" }, // IBM EBCDIC US-Canada (037 + Euro symbol); IBM EBCDIC (US-Canada-Euro)
      { 1141, "IBM01141" }, // IBM EBCDIC Germany (20273 + Euro symbol); IBM EBCDIC (Germany-Euro)
      { 1142, "IBM01142" }, // IBM EBCDIC Denmark-Norway (20277 + Euro symbol); IBM EBCDIC (Denmark-Norway-Euro)
      { 1143, "IBM01143" }, // IBM EBCDIC Finland-Sweden (20278 + Euro symbol); IBM EBCDIC (Finland-Sweden-Euro)
      { 1144, "IBM01144" }, // IBM EBCDIC Italy (20280 + Euro symbol); IBM EBCDIC (Italy-Euro)
      { 1145, "IBM01145" }, // IBM EBCDIC Latin America-Spain (20284 + Euro symbol); IBM EBCDIC (Spain-Euro)
      { 1146, "IBM01146" }, // IBM EBCDIC United Kingdom (20285 + Euro symbol); IBM EBCDIC (UK-Euro)
      { 1147, "IBM01147" }, // IBM EBCDIC France (20297 + Euro symbol); IBM EBCDIC (France-Euro)
      { 1148, "IBM01148" }, // IBM EBCDIC International (500 + Euro symbol); IBM EBCDIC (International-Euro)
      { 1149, "IBM01149" }, // IBM EBCDIC Icelandic (20871 + Euro symbol); IBM EBCDIC (Icelandic-Euro)
      { 1200, "utf-16" }, // Unicode UTF-16, little endian byte order (BMP of ISO 10646); available only to managed applications
      { 1201, "unicodeFFFE" }, // Unicode UTF-16, big endian byte order; available only to managed applications
      { 1250, "windows-1250" }, // ANSI Central European; Central European (Windows)
      { 1251, "windows-1251" }, // ANSI Cyrillic; Cyrillic (Windows)
      { 1252, "windows-1252" }, // ANSI Latin 1; Western European (Windows)
      { 1253, "windows-1253" }, // ANSI Greek; Greek (Windows)
      { 1254, "windows-1254" }, // ANSI Turkish; Turkish (Windows)
      { 1255, "windows-1255" }, // ANSI Hebrew; Hebrew (Windows)
      { 1256, "windows-1256" }, // ANSI Arabic; Arabic (Windows)
      { 1257, "windows-1257" }, // ANSI Baltic; Baltic (Windows)
      { 1258, "windows-1258" }, // ANSI/OEM Vietnamese; Vietnamese (Windows)
      { 1250, "win1250" }, // ANSI Central European; Central European (Windows)
      { 1251, "win1251" }, // ANSI Cyrillic; Cyrillic (Windows)
      { 1252, "win1252" }, // ANSI Latin 1; Western European (Windows)
      { 1253, "win1253" }, // ANSI Greek; Greek (Windows)
      { 1254, "win1254" }, // ANSI Turkish; Turkish (Windows)
      { 1255, "win1255" }, // ANSI Hebrew; Hebrew (Windows)
      { 1256, "win1256" }, // ANSI Arabic; Arabic (Windows)
      { 1257, "win1257" }, // ANSI Baltic; Baltic (Windows)
      { 1258, "win1258" }, // ANSI/OEM Vietnamese; Vietnamese (Windows)
      { 1361, "Johab" }, // Korean (Johab)
      { 10000, "macintosh" }, // MAC Roman; Western European (Mac)
      { 10001, "x-mac-japanese" }, // Japanese (Mac)
      { 10002, "x-mac-chinesetrad" }, // MAC Traditional Chinese (Big5); Chinese Traditional (Mac)
      { 10003, "x-mac-korean" }, // Korean (Mac)
      { 10004, "x-mac-arabic" }, // Arabic (Mac)
      { 10005, "x-mac-hebrew" }, // Hebrew (Mac)
      { 10006, "x-mac-greek" }, // Greek (Mac)
      { 10007, "x-mac-cyrillic" }, // Cyrillic (Mac)
      { 10008, "x-mac-chinesesimp" }, // MAC Simplified Chinese (GB 2312); Chinese Simplified (Mac)
      { 10010, "x-mac-romanian" }, // Romanian (Mac)
      { 10017, "x-mac-ukrainian" }, // Ukrainian (Mac)
      { 10021, "x-mac-thai" }, // Thai (Mac)
      { 10029, "x-mac-ce" }, // MAC Latin 2; Central European (Mac)
      { 10079, "x-mac-icelandic" }, // Icelandic (Mac)
      { 10081, "x-mac-turkish" }, // Turkish (Mac)
      { 10082, "x-mac-croatian" }, // Croatian (Mac)
      { 12000, "utf-32" }, // Unicode UTF-32, little endian byte order; available only to managed applications
      { 12001, "utf-32BE" }, // Unicode UTF-32, big endian byte order; available only to managed applications
      { 20000, "x-Chinese_CNS" }, // CNS Taiwan; Chinese Traditional (CNS)
      { 20001, "x-cp20001" }, // TCA Taiwan
      { 20002, "x_Chinese-Eten" }, // Eten Taiwan; Chinese Traditional (Eten)
      { 20003, "x-cp20003" }, // IBM5550 Taiwan
      { 20004, "x-cp20004" }, // TeleText Taiwan
      { 20005, "x-cp20005" }, // Wang Taiwan
      { 20105, "x-IA5" }, // IA5 (IRV International Alphabet No. 5, 7-bit); Western European (IA5)
      { 20106, "x-IA5-German" }, // IA5 German (7-bit)
      { 20107, "x-IA5-Swedish" }, // IA5 Swedish (7-bit)
      { 20108, "x-IA5-Norwegian" }, // IA5 Norwegian (7-bit)
      { 20127, "us-ascii" }, // US-ASCII (7-bit)
      { 20261, "x-cp20261" }, // T.61
      { 20269, "x-cp20269" }, // ISO 6937 Non-Spacing Accent
      { 20273, "IBM273" }, // IBM EBCDIC Germany
      { 20277, "IBM277" }, // IBM EBCDIC Denmark-Norway
      { 20278, "IBM278" }, // IBM EBCDIC Finland-Sweden
      { 20280, "IBM280" }, // IBM EBCDIC Italy
      { 20284, "IBM284" }, // IBM EBCDIC Latin America-Spain
      { 20285, "IBM285" }, // IBM EBCDIC United Kingdom
      { 20290, "IBM290" }, // IBM EBCDIC Japanese Katakana Extended
      { 20297, "IBM297" }, // IBM EBCDIC France
      { 20420, "IBM420" }, // IBM EBCDIC Arabic
      { 20423, "IBM423" }, // IBM EBCDIC Greek
      { 20424, "IBM424" }, // IBM EBCDIC Hebrew
      { 20833, "x-EBCDIC-KoreanExtended" }, // IBM EBCDIC Korean Extended
      { 20838, "IBM-Thai" }, // IBM EBCDIC Thai
      { 20866, "koi8-r" }, // Russian (KOI8-R); Cyrillic (KOI8-R)
      { 20871, "IBM871" }, // IBM EBCDIC Icelandic
      { 20880, "IBM880" }, // IBM EBCDIC Cyrillic Russian
      { 20905, "IBM905" }, // IBM EBCDIC Turkish
      { 20924, "IBM00924" }, // IBM EBCDIC Latin 1/Open System (1047 + Euro symbol)
      { 20932, "EUC-JP" }, // Japanese (JIS 0208-1990 and 0121-1990)
      { 20936, "x-cp20936" }, // Simplified Chinese (GB2312); Chinese Simplified (GB2312-80)
      { 20949, "x-cp20949" }, // Korean Wansung
      { 21025, "cp1025" }, // IBM EBCDIC Cyrillic Serbian-Bulgarian
      { 21027, "(deprecated)" }, //
      { 21866, "koi8-u" }, // Ukrainian (KOI8-U); Cyrillic (KOI8-U)
      { 28600, "iso-8859-10" }, // ISO 8859-10
      { 28601, "iso-8859-11" }, // ISO 8859-11
      { 28602, "iso-8859-12" }, // ISO 8859-12
      { 28603, "iso-8859-13" }, // ISO 8859-13 Estonian
      { 28604, "iso-8859-14" }, // ISO 8859-14
      { 28605, "iso-8859-15" }, // ISO 8859-15 Latin 9
      { 28606, "iso-8859-16" }, // ISO 8859-16
      { 28591, "iso-8859-1" }, // ISO 8859-1 Latin 1; Western European (ISO)
      { 28592, "iso-8859-2" }, // ISO 8859-2 Central European; Central European (ISO)
      { 28593, "iso-8859-3" }, // ISO 8859-3 Latin 3
      { 28594, "iso-8859-4" }, // ISO 8859-4 Baltic
      { 28595, "iso-8859-5" }, // ISO 8859-5 Cyrillic
      { 28596, "iso-8859-6" }, // ISO 8859-6 Arabic
      { 28597, "iso-8859-7" }, // ISO 8859-7 Greek
      { 28598, "iso-8859-8" }, // ISO 8859-8 Hebrew; Hebrew (ISO-Visual)
      { 28599, "iso-8859-9" }, // ISO 8859-9 Turkish
      { 29001, "x-Europa" }, // Europa 3
      { 38598, "iso-8859-8-i" }, // ISO 8859-8 Hebrew; Hebrew (ISO-Logical)
      { 50220, "iso-2022-jp" }, // ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS)
      { 50221, "csISO2022JP" }, // ISO 2022 Japanese with halfwidth Katakana; Japanese (JIS-Allow 1 byte Kana)
      { 50222, "iso-2022-jp" }, // ISO 2022 Japanese JIS X 0201-1989; Japanese (JIS-Allow 1 byte Kana - SO/SI)
      { 50225, "iso-2022-kr" }, // ISO 2022 Korean
      { 50227, "x-cp50227" }, // ISO 2022 Simplified Chinese; Chinese Simplified (ISO 2022)
      { 50229, "ISO" }, // 2022 Traditional Chinese
      { 50930, "EBCDIC-jp" }, // Japanese (Katakana) Extended
      { 50931, "EBCDIC-us" }, // US-Canada and Japanese
      { 50933, "EBCDIC-kr" }, // Korean Extended and Korean
      { 50935, "EBCDIC-sx" }, // Simplified Chinese Extended and Simplified Chinese
      { 50936, "EBCDIC-sc" }, // Simplified Chinese
      { 50937, "EBCDIC-uc" }, // US-Canada and Traditional Chinese
      { 50939, "EBCDIC-jl" }, // Japanese (Latin) Extended and Japanese
      { 51932, "euc-jp" }, // EUC Japanese
      { 51936, "EUC-CN" }, // EUC Simplified Chinese; Chinese Simplified (EUC)
      { 51949, "euc-kr" }, // EUC Korean
      { 51950, "EUC" }, // Traditional Chinese
      { 52936, "hz-gb-2312" }, // HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ)
      { 54936, "GB18030" }, // Windows XP and later: GB18030 Simplified Chinese (4 byte); Chinese Simplified (GB18030)
      { 57002, "x-iscii-de" }, // ISCII Devanagari
      { 57003, "x-iscii-be" }, // ISCII Bengali
      { 57004, "x-iscii-ta" }, // ISCII Tamil
      { 57005, "x-iscii-te" }, // ISCII Telugu
      { 57006, "x-iscii-as" }, // ISCII Assamese
      { 57007, "x-iscii-or" }, // ISCII Oriya
      { 57008, "x-iscii-ka" }, // ISCII Kannada
      { 57009, "x-iscii-ma" }, // ISCII Malayalam
      { 57010, "x-iscii-gu" }, // ISCII Gujarati
      { 57011, "x-iscii-pa" }, // ISCII Punjabi
      { 65000, "utf-7" }, // Unicode (UTF-7)
      { 65001, "utf-8" }, // Unicode (UTF-8)
      { 65001, 0 } // delimiter
  };

  /**
   * Get the code page from selected name.
   */
  void setCodePage(struct Sftp4tc * c) {
    struct CpData * cp = &CPDATA[0];

	char * line_codepage = conf_get_str(c->config, CONF_line_codepage);
    // search all code page
    while (cp->name) {
      char * p = line_codepage;
      char * q = cp->name;
      while (*q) {
        if (tolower(*p) != tolower(*q))
          break;
        ++p;
        ++q;
      }
      if (!*q)
        break;
      ++cp;
    }
    c->codePage = cp->cp;
  }

    /**
   * Get the code page from selected name.
   */
  char * getCodePage(struct Sftp4tc * c) {
    struct CpData * cp = &CPDATA[0];

    // search all code page
    while (cp->name) {
		if (c->codePage == cp->cp)
			return cp->name;
      ++cp;
    }
    return NULL;
  }


/*
* Convenience functions to access the backends[] array
* (which is only present in tools that manage settings).
*/

Backend *backend_from_name(const char *name)
{
	Backend **p;
	for (p = backends; *p != NULL; p++)
		if (!strcmp((*p)->name, name))
			return *p;
	return NULL;
}

Backend *backend_from_proto(int proto)
{
	Backend **p;
	for (p = backends; *p != NULL; p++)
		if ((*p)->protocol == proto)
			return *p;
	return NULL;
}

char *get_remote_username(Conf *conf)
{
	char *username = conf_get_str(conf, CONF_username);
	if (*username) {
		return dupstr(username);
	} else if (conf_get_int(conf, CONF_username_from_env)) {
		/* Use local username. */
		return get_username();     /* might still be NULL */
	} else {
		return NULL;
	}
}

static char *gpps_raw(struct KeyOrIni *handle, const char *name, const char *def)
{
	char *ret = read_setting_s(handle, name);
	if (!ret)
		ret = platform_default_s(name);
	if (!ret)
		ret = def ? dupstr(def) : NULL;   /* permit NULL as final fallback */
	return ret;
}

static void gpps(struct KeyOrIni *handle, const char *name, const char *def,
				 Conf *conf, int primary)
{
	char *val = gpps_raw(handle, name, def);
	conf_set_str(conf, primary, val);
	sfree(val);
}

/*
* gppfont and gppfile cannot have local defaults, since the very
* format of a Filename or FontSpec is platform-dependent. So the
* platform-dependent functions MUST return some sort of value.
*/
static void gppfont(struct KeyOrIni *handle, const char *name, Conf *conf, int primary)
{
	FontSpec *result = read_setting_fontspec(handle, name);
	if (!result)
		result = platform_default_fontspec(name);
	conf_set_fontspec(conf, primary, result);
	fontspec_free(result);
}
static void gppfile(struct KeyOrIni *handle, const char *name, Conf *conf, int primary)
{
	Filename *result = read_setting_filename(handle, name);
	if (!result)
		result = platform_default_filename(name);
	conf_set_filename(conf, primary, result);
	filename_free(result);
}

static int gppi_raw(struct KeyOrIni *handle, char *name, int def)
{
	def = platform_default_i(name, def);
	return read_setting_i(handle, name, def);
}

static void gppi(struct KeyOrIni *handle, char *name, int def, Conf *conf, int primary)
{
	conf_set_int(conf, primary, gppi_raw(handle, name, def));
}

/*
* Read a set of name-value pairs in the format we occasionally use:
*   NAME\tVALUE\0NAME\tVALUE\0\0 in memory
*   NAME=VALUE,NAME=VALUE, in storage
* `def' is in the storage format.
*/
static int gppmap(void *handle, char *name, Conf *conf, int primary)
{
	char *buf, *p, *q, *key, *val;

	/*
	* Start by clearing any existing subkeys of this key from conf.
	*/
	while ((key = conf_get_str_nthstrkey(conf, primary, 0)) != NULL)
		conf_del_str_str(conf, primary, key);

	/*
	* Now read a serialised list from the settings and unmarshal it
	* into its components.
	*/
	buf = gpps_raw(handle, name, NULL);
	if (!buf)
		return FALSE;

	p = buf;
	while (*p) {
		q = buf;
		val = NULL;
		while (*p && *p != ',') {
			int c = *p++;
			if (c == '=')
				c = '\0';
			if (c == '\\')
				c = *p++;
			*q++ = c;
			if (!c)
				val = q;
		}
		if (*p == ',')
			p++;
		if (!val)
			val = q;
		*q = '\0';

		if (primary == CONF_portfwd && buf[0] == 'D') {
			/*
			* Backwards-compatibility hack: dynamic forwardings are
			* indexed in the data store as a third type letter in the
			* key, 'D' alongside 'L' and 'R' - but really, they
			* should be filed under 'L' with a special _value_,
			* because local and dynamic forwardings both involve
			* _listening_ on a local port, and are hence mutually
			* exclusive on the same port number. So here we translate
			* the legacy storage format into the sensible internal
			* form.
			*/
			char *newkey = dupcat("L", buf+1, NULL);
			conf_set_str_str(conf, primary, newkey, "D");
			sfree(newkey);
		} else {
			conf_set_str_str(conf, primary, buf, val);
		}
	}
	sfree(buf);

	return TRUE;
}

/*
* Write a set of name/value pairs in the above format.
*/
static void wmap(void *handle, char const *outkey, Conf *conf, int primary)
{
	char *buf, *p, *q, *key, *realkey, *val;
	int len;

	len = 1;			       /* allow for NUL */

	for (val = conf_get_str_strs(conf, primary, NULL, &key);
		val != NULL;
		val = conf_get_str_strs(conf, primary, key, &key))
		len += 2 + 2 * ((int)strlen(key) + (int)strlen(val));   /* allow for escaping */

	buf = snewn(len, char);
	p = buf;

	for (val = conf_get_str_strs(conf, primary, NULL, &key);
		val != NULL;
		val = conf_get_str_strs(conf, primary, key, &key)) {

			if (primary == CONF_portfwd && !strcmp(val, "D")) {
				/*
				* Backwards-compatibility hack, as above: translate from
				* the sensible internal representation of dynamic
				* forwardings (key "L<port>", value "D") to the
				* conceptually incoherent legacy storage format (key
				* "D<port>", value empty).
				*/
				realkey = key;             /* restore it at end of loop */
				val = "";
				key = dupcat("D", key+1, NULL);
			} else {
				realkey = NULL;
			}

			if (p != buf)
				*p++ = ',';
			for (q = key; *q; q++) {
				if (*q == '=' || *q == ',' || *q == '\\')
					*p++ = '\\';
				*p++ = *q;
			}
			*p++ = '=';
			for (q = val; *q; q++) {
				if (*q == '=' || *q == ',' || *q == '\\')
					*p++ = '\\';
				*p++ = *q;
			}

			if (realkey) {
				free(key);
				key = realkey;
			}
	}
	*p = '\0';
	write_setting_s(handle, outkey, buf);
	sfree(buf);
}

static int key2val(const struct keyvalwhere *mapping,
				   int nmaps, char *key)
{
	int i;
	for (i = 0; i < nmaps; i++)
		if (!strcmp(mapping[i].s, key)) return mapping[i].v;
	return -1;
}

static const char *val2key(const struct keyvalwhere *mapping,
						   int nmaps, int val)
{
	int i;
	for (i = 0; i < nmaps; i++)
		if (mapping[i].v == val) return mapping[i].s;
	return NULL;
}

/*
* Helper function to parse a comma-separated list of strings into
* a preference list array of values. Any missing values are added
* to the end and duplicates are weeded.
* XXX: assumes vals in 'mapping' are small +ve integers
*/
static void gprefs(void *sesskey, char *name, char *def,
				   const struct keyvalwhere *mapping, int nvals,
				   Conf *conf, int primary)
{
	char *commalist;
	char *p, *q;
	int i, j, n, v, pos;
	unsigned long seen = 0;	       /* bitmap for weeding dups etc */

	/*
	* Fetch the string which we'll parse as a comma-separated list.
	*/
	commalist = gpps_raw(sesskey, name, def);

	/*
	* Go through that list and convert it into values.
	*/
	n = 0;
	p = commalist;
	while (1) {
		while (*p && *p == ',') p++;
		if (!*p)
			break;                     /* no more words */

		q = p;
		while (*p && *p != ',') p++;
		if (*p) *p++ = '\0';

		v = key2val(mapping, nvals, q);
		if (v != -1 && !(seen & (1 << v))) {
			seen |= (1 << v);
			conf_set_int_int(conf, primary, n, v);
			n++;
		}
	}

	sfree(commalist);

	/*
	* Now go through 'mapping' and add values that weren't mentioned
	* in the list we fetched. We may have to loop over it multiple
	* times so that we add values before other values whose default
	* positions depend on them.
	*/
	while (n < nvals) {
		for (i = 0; i < nvals; i++) {
			assert(mapping[i].v < 32);

			if (!(seen & (1 << mapping[i].v))) {
				/*
				* This element needs adding. But can we add it yet?
				*/
				if (mapping[i].vrel != -1 && !(seen & (1 << mapping[i].vrel)))
					continue;          /* nope */

				/*
				* OK, we can work out where to add this element, so
				* do so.
				*/
				if (mapping[i].vrel == -1) {
					pos = (mapping[i].where < 0 ? n : 0);
				} else {
					for (j = 0; j < n; j++)
						if (conf_get_int_int(conf, primary, j) ==
							mapping[i].vrel)
							break;
					assert(j < n);     /* implied by (seen & (1<<vrel)) */
					pos = (mapping[i].where < 0 ? j : j+1);
				}

				/*
				* And add it.
				*/
				for (j = n-1; j >= pos; j--)
					conf_set_int_int(conf, primary, j+1,
					conf_get_int_int(conf, primary, j));
				conf_set_int_int(conf, primary, pos, mapping[i].v);
				n++;
			}
		}
	}
}

/* 
* Write out a preference list.
*/
static void wprefs(struct KeyOrIni *sesskey, char *name,
				   const struct keyvalwhere *mapping, int nvals,
				   Conf *conf, int primary)
{
	char *buf, *p;
	int i, maxlen;

	for (maxlen = i = 0; i < nvals; i++) {
		const char *s = val2key(mapping, nvals,
			conf_get_int_int(conf, primary, i));
		if (s) {
			maxlen += (maxlen > 0 ? 1 : 0) + (int)strlen(s);
		}
	}

	buf = snewn(maxlen + 1, char);
	p = buf;

	for (i = 0; i < nvals; i++) {
		const char *s = val2key(mapping, nvals,
			conf_get_int_int(conf, primary, i));
		if (s) {
			p += sprintf(p, "%s%s", (p > buf ? "," : ""), s);
		}
	}

	assert(p - buf == maxlen);
	*p = '\0';

	write_setting_s(sesskey, name, buf);

	sfree(buf);
}

char *save_settings(char *section, struct Sftp4tc *conf)
{
	struct KeyOrIni *sesskey;
	char *errmsg;

	sesskey = open_settings_w(section, conf, &errmsg);
	if (!sesskey)
		return errmsg;
	save_open_settings(sesskey, conf->config);
	close_settings_w(sesskey);
	return NULL;
}

void save_open_settings(struct KeyOrIni *sesskey, Conf *conf)
{
	int i;
	char *p;

	write_setting_i(sesskey, "Present", 1);
	write_setting_s(sesskey, "HostName", conf_get_str(conf, CONF_host));
	write_setting_filename(sesskey, "LogFileName", conf_get_filename(conf, CONF_logfilename));
	write_setting_i(sesskey, "LogType", conf_get_int(conf, CONF_logtype));
	write_setting_i(sesskey, "LogFileClash", conf_get_int(conf, CONF_logxfovr));
	write_setting_i(sesskey, "LogFlush", conf_get_int(conf, CONF_logflush));
	write_setting_i(sesskey, "SSHLogOmitPasswords", conf_get_int(conf, CONF_logomitpass));
	write_setting_i(sesskey, "SSHLogOmitData", conf_get_int(conf, CONF_logomitdata));
	p = "raw";
	{
		const Backend *b = backend_from_proto(conf_get_int(conf, CONF_protocol));
		if (b)
			p = b->name;
	}
	write_setting_s(sesskey, "Protocol", p);
	write_setting_i(sesskey, "PortNumber", conf_get_int(conf, CONF_port));
	/* The CloseOnExit numbers are arranged in a different order from
	* the standard FORCE_ON / FORCE_OFF / AUTO. */
	write_setting_i(sesskey, "CloseOnExit", (conf_get_int(conf, CONF_close_on_exit)+2)%3);
	write_setting_i(sesskey, "WarnOnClose", !!conf_get_int(conf, CONF_warn_on_close));
	write_setting_i(sesskey, "PingInterval", conf_get_int(conf, CONF_ping_interval) / 60);	/* minutes */
	write_setting_i(sesskey, "PingIntervalSecs", conf_get_int(conf, CONF_ping_interval) % 60);	/* seconds */
	write_setting_i(sesskey, "TCPNoDelay", conf_get_int(conf, CONF_tcp_nodelay));
	write_setting_i(sesskey, "TCPKeepalives", conf_get_int(conf, CONF_tcp_keepalives));
	write_setting_s(sesskey, "TerminalType", conf_get_str(conf, CONF_termtype));
	write_setting_s(sesskey, "TerminalSpeed", conf_get_str(conf, CONF_termspeed));
	wmap(sesskey, "TerminalModes", conf, CONF_ttymodes);

	/* Address family selection */
	write_setting_i(sesskey, "AddressFamily", conf_get_int(conf, CONF_addressfamily));

	/* proxy settings */
	write_setting_s(sesskey, "ProxyExcludeList", conf_get_str(conf, CONF_proxy_exclude_list));
	write_setting_i(sesskey, "ProxyDNS", (conf_get_int(conf, CONF_proxy_dns)+2)%3);
	write_setting_i(sesskey, "ProxyLocalhost", conf_get_int(conf, CONF_even_proxy_localhost));
	write_setting_i(sesskey, "ProxyMethod", conf_get_int(conf, CONF_proxy_type));
	write_setting_s(sesskey, "ProxyHost", conf_get_str(conf, CONF_proxy_host));
	write_setting_i(sesskey, "ProxyPort", conf_get_int(conf, CONF_proxy_port));
	write_setting_s(sesskey, "ProxyUsername", conf_get_str(conf, CONF_proxy_username));
	write_setting_s(sesskey, "ProxyPassword", conf_get_str(conf, CONF_proxy_password));
	write_setting_s(sesskey, "ProxyTelnetCommand", conf_get_str(conf, CONF_proxy_telnet_command));
	wmap(sesskey, "Environment", conf, CONF_environmt);
	write_setting_s(sesskey, "UserName", conf_get_str(conf, CONF_username));
	write_setting_i(sesskey, "UserNameFromEnvironment", conf_get_int(conf, CONF_username_from_env));
	write_setting_s(sesskey, "LocalUserName", conf_get_str(conf, CONF_localusername));
	write_setting_i(sesskey, "NoPTY", conf_get_int(conf, CONF_nopty));
	write_setting_i(sesskey, "Compression", conf_get_int(conf, CONF_compression));
	write_setting_i(sesskey, "TryAgent", conf_get_int(conf, CONF_tryagent));
	write_setting_i(sesskey, "AgentFwd", conf_get_int(conf, CONF_agentfwd));
	write_setting_i(sesskey, "GssapiFwd", conf_get_int(conf, CONF_gssapifwd));
	write_setting_i(sesskey, "ChangeUsername", conf_get_int(conf, CONF_change_username));
	wprefs(sesskey, "Cipher", ciphernames, CIPHER_MAX, conf, CONF_ssh_cipherlist);
	wprefs(sesskey, "KEX", kexnames, KEX_MAX, conf, CONF_ssh_kexlist);
	write_setting_i(sesskey, "RekeyTime", conf_get_int(conf, CONF_ssh_rekey_time));
	write_setting_s(sesskey, "RekeyBytes", conf_get_str(conf, CONF_ssh_rekey_data));
	write_setting_i(sesskey, "SshNoAuth", conf_get_int(conf, CONF_ssh_no_userauth));
	write_setting_i(sesskey, "SshBanner", conf_get_int(conf, CONF_ssh_show_banner));
	write_setting_i(sesskey, "AuthTIS", conf_get_int(conf, CONF_try_tis_auth));
	write_setting_i(sesskey, "AuthKI", conf_get_int(conf, CONF_try_ki_auth));
	write_setting_i(sesskey, "AuthGSSAPI", conf_get_int(conf, CONF_try_gssapi_auth));
#ifndef NO_GSSAPI
	wprefs(sesskey, "GSSLibs", gsslibkeywords, ngsslibs, conf, CONF_ssh_gsslist);
	write_setting_filename(sesskey, "GSSCustom", conf_get_filename(conf, CONF_ssh_gss_custom));
#endif
	write_setting_i(sesskey, "SshNoShell", conf_get_int(conf, CONF_ssh_no_shell));
	write_setting_i(sesskey, "SshProt", conf_get_int(conf, CONF_sshprot));
	write_setting_s(sesskey, "LogHost", conf_get_str(conf, CONF_loghost));
	write_setting_i(sesskey, "SSH2DES", conf_get_int(conf, CONF_ssh2_des_cbc));
	write_setting_filename(sesskey, "PublicKeyFile", conf_get_filename(conf, CONF_keyfile));
	write_setting_s(sesskey, "RemoteCommand", conf_get_str(conf, CONF_remote_cmd));
	write_setting_i(sesskey, "RFCEnviron", conf_get_int(conf, CONF_rfc_environ));
	write_setting_i(sesskey, "PassiveTelnet", conf_get_int(conf, CONF_passive_telnet));
	write_setting_i(sesskey, "BackspaceIsDelete", conf_get_int(conf, CONF_bksp_is_delete));
	write_setting_i(sesskey, "RXVTHomeEnd", conf_get_int(conf, CONF_rxvt_homeend));
	write_setting_i(sesskey, "LinuxFunctionKeys", conf_get_int(conf, CONF_funky_type));
	write_setting_i(sesskey, "NoApplicationKeys", conf_get_int(conf, CONF_no_applic_k));
	write_setting_i(sesskey, "NoApplicationCursors", conf_get_int(conf, CONF_no_applic_c));
	write_setting_i(sesskey, "NoMouseReporting", conf_get_int(conf, CONF_no_mouse_rep));
	write_setting_i(sesskey, "NoRemoteResize", conf_get_int(conf, CONF_no_remote_resize));
	write_setting_i(sesskey, "NoAltScreen", conf_get_int(conf, CONF_no_alt_screen));
	write_setting_i(sesskey, "NoRemoteWinTitle", conf_get_int(conf, CONF_no_remote_wintitle));
	write_setting_i(sesskey, "RemoteQTitleAction", conf_get_int(conf, CONF_remote_qtitle_action));
	write_setting_i(sesskey, "NoDBackspace", conf_get_int(conf, CONF_no_dbackspace));
	write_setting_i(sesskey, "NoRemoteCharset", conf_get_int(conf, CONF_no_remote_charset));
	write_setting_i(sesskey, "ApplicationCursorKeys", conf_get_int(conf, CONF_app_cursor));
	write_setting_i(sesskey, "ApplicationKeypad", conf_get_int(conf, CONF_app_keypad));
	write_setting_i(sesskey, "NetHackKeypad", conf_get_int(conf, CONF_nethack_keypad));
	write_setting_i(sesskey, "AltF4", conf_get_int(conf, CONF_alt_f4));
	write_setting_i(sesskey, "AltSpace", conf_get_int(conf, CONF_alt_space));
	write_setting_i(sesskey, "AltOnly", conf_get_int(conf, CONF_alt_only));
	write_setting_i(sesskey, "ComposeKey", conf_get_int(conf, CONF_compose_key));
	write_setting_i(sesskey, "CtrlAltKeys", conf_get_int(conf, CONF_ctrlaltkeys));
	write_setting_i(sesskey, "TelnetKey", conf_get_int(conf, CONF_telnet_keyboard));
	write_setting_i(sesskey, "TelnetRet", conf_get_int(conf, CONF_telnet_newline));
	write_setting_i(sesskey, "LocalEcho", conf_get_int(conf, CONF_localecho));
	write_setting_i(sesskey, "LocalEdit", conf_get_int(conf, CONF_localedit));
	write_setting_s(sesskey, "Answerback", conf_get_str(conf, CONF_answerback));
	write_setting_i(sesskey, "AlwaysOnTop", conf_get_int(conf, CONF_alwaysontop));
	write_setting_i(sesskey, "FullScreenOnAltEnter", conf_get_int(conf, CONF_fullscreenonaltenter));
	write_setting_i(sesskey, "HideMousePtr", conf_get_int(conf, CONF_hide_mouseptr));
	write_setting_i(sesskey, "SunkenEdge", conf_get_int(conf, CONF_sunken_edge));
	write_setting_i(sesskey, "WindowBorder", conf_get_int(conf, CONF_window_border));
	write_setting_i(sesskey, "CurType", conf_get_int(conf, CONF_cursor_type));
	write_setting_i(sesskey, "BlinkCur", conf_get_int(conf, CONF_blink_cur));
	write_setting_i(sesskey, "Beep", conf_get_int(conf, CONF_beep));
	write_setting_i(sesskey, "BeepInd", conf_get_int(conf, CONF_beep_ind));
	write_setting_filename(sesskey, "BellWaveFile", conf_get_filename(conf, CONF_bell_wavefile));
	write_setting_i(sesskey, "BellOverload", conf_get_int(conf, CONF_bellovl));
	write_setting_i(sesskey, "BellOverloadN", conf_get_int(conf, CONF_bellovl_n));
	write_setting_i(sesskey, "BellOverloadT", conf_get_int(conf, CONF_bellovl_t)
#ifdef PUTTY_UNIX_H
		* 1000
#endif
		);
	write_setting_i(sesskey, "BellOverloadS", conf_get_int(conf, CONF_bellovl_s)
#ifdef PUTTY_UNIX_H
		* 1000
#endif
		);
	write_setting_i(sesskey, "ScrollbackLines", conf_get_int(conf, CONF_savelines));
	write_setting_i(sesskey, "DECOriginMode", conf_get_int(conf, CONF_dec_om));
	write_setting_i(sesskey, "AutoWrapMode", conf_get_int(conf, CONF_wrap_mode));
	write_setting_i(sesskey, "LFImpliesCR", conf_get_int(conf, CONF_lfhascr));
	write_setting_i(sesskey, "CRImpliesLF", conf_get_int(conf, CONF_crhaslf));
	write_setting_i(sesskey, "DisableArabicShaping", conf_get_int(conf, CONF_arabicshaping));
	write_setting_i(sesskey, "DisableBidi", conf_get_int(conf, CONF_bidi));
	write_setting_i(sesskey, "WinNameAlways", conf_get_int(conf, CONF_win_name_always));
	write_setting_s(sesskey, "WinTitle", conf_get_str(conf, CONF_wintitle));
	write_setting_i(sesskey, "TermWidth", conf_get_int(conf, CONF_width));
	write_setting_i(sesskey, "TermHeight", conf_get_int(conf, CONF_height));
	write_setting_fontspec(sesskey, "Font", conf_get_fontspec(conf, CONF_font));
	write_setting_i(sesskey, "FontQuality", conf_get_int(conf, CONF_font_quality));
	write_setting_i(sesskey, "FontVTMode", conf_get_int(conf, CONF_vtmode));
	write_setting_i(sesskey, "UseSystemColours", conf_get_int(conf, CONF_system_colour));
	write_setting_i(sesskey, "TryPalette", conf_get_int(conf, CONF_try_palette));
	write_setting_i(sesskey, "ANSIColour", conf_get_int(conf, CONF_ansi_colour));
	write_setting_i(sesskey, "Xterm256Colour", conf_get_int(conf, CONF_xterm_256_colour));
	write_setting_i(sesskey, "BoldAsColour", conf_get_int(conf, CONF_bold_style)-1);

	for (i = 0; i < 22; i++) {
		char buf[20], buf2[30];
		sprintf(buf, "Colour%d", i);
		sprintf(buf2, "%d,%d,%d",
			conf_get_int_int(conf, CONF_colours, i*3+0),
			conf_get_int_int(conf, CONF_colours, i*3+1),
			conf_get_int_int(conf, CONF_colours, i*3+2));
		write_setting_s(sesskey, buf, buf2);
	}
	write_setting_i(sesskey, "RawCNP", conf_get_int(conf, CONF_rawcnp));
	write_setting_i(sesskey, "PasteRTF", conf_get_int(conf, CONF_rtf_paste));
	write_setting_i(sesskey, "MouseIsXterm", conf_get_int(conf, CONF_mouse_is_xterm));
	write_setting_i(sesskey, "RectSelect", conf_get_int(conf, CONF_rect_select));
	write_setting_i(sesskey, "MouseOverride", conf_get_int(conf, CONF_mouse_override));
	for (i = 0; i < 256; i += 32) {
		char buf[20], buf2[256];
		int j;
		sprintf(buf, "Wordness%d", i);
		*buf2 = '\0';
		for (j = i; j < i + 32; j++) {
			sprintf(buf2 + strlen(buf2), "%s%d",
				(*buf2 ? "," : ""),
				conf_get_int_int(conf, CONF_wordness, j));
		}
		write_setting_s(sesskey, buf, buf2);
	}
	write_setting_s(sesskey, "LineCodePage", conf_get_str(conf, CONF_line_codepage));
	write_setting_i(sesskey, "CJKAmbigWide", conf_get_int(conf, CONF_cjk_ambig_wide));
	write_setting_i(sesskey, "UTF8Override", conf_get_int(conf, CONF_utf8_override));
	write_setting_s(sesskey, "Printer", conf_get_str(conf, CONF_printer));
	write_setting_i(sesskey, "CapsLockCyr", conf_get_int(conf, CONF_xlat_capslockcyr));
	write_setting_i(sesskey, "ScrollBar", conf_get_int(conf, CONF_scrollbar));
	write_setting_i(sesskey, "ScrollBarFullScreen", conf_get_int(conf, CONF_scrollbar_in_fullscreen));
	write_setting_i(sesskey, "ScrollOnKey", conf_get_int(conf, CONF_scroll_on_key));
	write_setting_i(sesskey, "ScrollOnDisp", conf_get_int(conf, CONF_scroll_on_disp));
	write_setting_i(sesskey, "EraseToScrollback", conf_get_int(conf, CONF_erase_to_scrollback));
	write_setting_i(sesskey, "LockSize", conf_get_int(conf, CONF_resize_action));
	write_setting_i(sesskey, "BCE", conf_get_int(conf, CONF_bce));
	write_setting_i(sesskey, "BlinkText", conf_get_int(conf, CONF_blinktext));
	write_setting_i(sesskey, "X11Forward", conf_get_int(conf, CONF_x11_forward));
	write_setting_s(sesskey, "X11Display", conf_get_str(conf, CONF_x11_display));
	write_setting_i(sesskey, "X11AuthType", conf_get_int(conf, CONF_x11_auth));
	write_setting_filename(sesskey, "X11AuthFile", conf_get_filename(conf, CONF_xauthfile));
	write_setting_i(sesskey, "LocalPortAcceptAll", conf_get_int(conf, CONF_lport_acceptall));
	write_setting_i(sesskey, "RemotePortAcceptAll", conf_get_int(conf, CONF_rport_acceptall));
	wmap(sesskey, "PortForwardings", conf, CONF_portfwd);
	write_setting_i(sesskey, "BugIgnore1", 2-conf_get_int(conf, CONF_sshbug_ignore1));
	write_setting_i(sesskey, "BugPlainPW1", 2-conf_get_int(conf, CONF_sshbug_plainpw1));
	write_setting_i(sesskey, "BugRSA1", 2-conf_get_int(conf, CONF_sshbug_rsa1));
	write_setting_i(sesskey, "BugIgnore2", 2-conf_get_int(conf, CONF_sshbug_ignore2));
	write_setting_i(sesskey, "BugHMAC2", 2-conf_get_int(conf, CONF_sshbug_hmac2));
	write_setting_i(sesskey, "BugDeriveKey2", 2-conf_get_int(conf, CONF_sshbug_derivekey2));
	write_setting_i(sesskey, "BugRSAPad2", 2-conf_get_int(conf, CONF_sshbug_rsapad2));
	write_setting_i(sesskey, "BugPKSessID2", 2-conf_get_int(conf, CONF_sshbug_pksessid2));
	write_setting_i(sesskey, "BugRekey2", 2-conf_get_int(conf, CONF_sshbug_rekey2));
	write_setting_i(sesskey, "BugMaxPkt2", 2-conf_get_int(conf, CONF_sshbug_maxpkt2));
	write_setting_i(sesskey, "BugWinadj", 2-conf_get_int(conf, CONF_sshbug_winadj));
	write_setting_i(sesskey, "StampUtmp", conf_get_int(conf, CONF_stamp_utmp));
	write_setting_i(sesskey, "LoginShell", conf_get_int(conf, CONF_login_shell));
	write_setting_i(sesskey, "ScrollbarOnLeft", conf_get_int(conf, CONF_scrollbar_on_left));
	write_setting_fontspec(sesskey, "BoldFont", conf_get_fontspec(conf, CONF_boldfont));
	write_setting_fontspec(sesskey, "WideFont", conf_get_fontspec(conf, CONF_widefont));
	write_setting_fontspec(sesskey, "WideBoldFont", conf_get_fontspec(conf, CONF_wideboldfont));
	write_setting_i(sesskey, "ShadowBold", conf_get_int(conf, CONF_shadowbold));
	write_setting_i(sesskey, "ShadowBoldOffset", conf_get_int(conf, CONF_shadowboldoffset));
	write_setting_s(sesskey, "SerialLine", conf_get_str(conf, CONF_serline));
	write_setting_i(sesskey, "SerialSpeed", conf_get_int(conf, CONF_serspeed));
	write_setting_i(sesskey, "SerialDataBits", conf_get_int(conf, CONF_serdatabits));
	write_setting_i(sesskey, "SerialStopHalfbits", conf_get_int(conf, CONF_serstopbits));
	write_setting_i(sesskey, "SerialParity", conf_get_int(conf, CONF_serparity));
	write_setting_i(sesskey, "SerialFlowControl", conf_get_int(conf, CONF_serflow));
	write_setting_s(sesskey, "WindowClass", conf_get_str(conf, CONF_winclass));

	// SFTP4TC
	write_setting_i(sesskey, "SFTP4TC:cacheFolders", conf_get_int(conf, CONF_sftpCacheFolders));
	write_setting_i(sesskey, "SFTP4TC:hideDotNames", conf_get_int(conf, CONF_sftpHideDotNames));
	write_setting_s(sesskey, "SFTP4TC:defChMod", conf_get_str(conf, CONF_sftpDefChMod));
	write_setting_s(sesskey, "SFTP4TC:exeChMod", conf_get_str(conf, CONF_sftpExeChMod));
	write_setting_s(sesskey, "SFTP4TC:exeExtensions", conf_get_str(conf, CONF_sftpExeExtensions));
	write_setting_s(sesskey, "SFTP4TC:homeDir", conf_get_str(conf, CONF_sftpHomeDir));
	write_setting_s(sesskey, "SFTP4TC:sftpCommand", conf_get_str(conf, CONF_sftpCommand));
	write_setting_i(sesskey, "SFTP4TC:storePassword", conf_get_int(conf, CONF_sftpStorePassword));
	
}

void load_settings(char *section, struct Sftp4tc *conf)
{
	struct KeyOrIni *sesskey;

	if (!conf->config) {
	  conf->config = conf_new();
//R	  conf_set_str(conf->config, CONF_host, conf->host);
    }

	sesskey = open_settings_r(section, conf);
	load_open_settings(sesskey, conf->config);
	close_settings_r(sesskey);

	if (conf_launchable(conf->config))
		add_session_to_jumplist(section);
}

void updateSftpCfg(struct Sftp4tc *conf) {
		  // update structure

  conf->cacheFolders = conf_get_int(conf->config, CONF_sftpCacheFolders);
  conf->hideDotNames = conf_get_int(conf->config, CONF_sftpHideDotNames);
  conf->port = conf_get_int(conf->config, CONF_port);
  strncpy(conf->defChMod, conf_get_str(conf->config, CONF_sftpDefChMod), 31);
  strncpy(conf->exeChMod, conf_get_str(conf->config, CONF_sftpExeChMod), 31);
  strncpy(conf->exeExtensions, conf_get_str(conf->config, CONF_sftpExeExtensions), 1023);
  strncpy(conf->homeDir, conf_get_str(conf->config, CONF_sftpHomeDir), 1023);
  strncpy(conf->host, conf_get_str(conf->config, CONF_host), 511);
  strncpy(conf->iniPath, conf_get_str(conf->config, CONF_iniPath), 1023);
  strncpy(conf->sftpCommand, conf_get_str(conf->config, CONF_sftpCommand), 1023);
  conf->storePassword = conf_get_int(conf->config, CONF_sftpStorePassword);

  setCodePage(conf);
}

void load_open_settings(struct KeyOrIni *sesskey, Conf *conf)
{
	int i;
	char *prot;

	conf_set_int(conf, CONF_ssh_subsys, 0);   /* FIXME: load this properly */
	conf_set_str(conf, CONF_remote_cmd, "");
	conf_set_str(conf, CONF_remote_cmd2, "");
	conf_set_str(conf, CONF_ssh_nc_host, "");

	gpps(sesskey, "HostName", "", conf, CONF_host);
	gppfile(sesskey, "LogFileName", conf, CONF_logfilename);
	gppi(sesskey, "LogType", 0, conf, CONF_logtype);
	gppi(sesskey, "LogFileClash", LGXF_ASK, conf, CONF_logxfovr);
	gppi(sesskey, "LogFlush", 1, conf, CONF_logflush);
	gppi(sesskey, "SSHLogOmitPasswords", 1, conf, CONF_logomitpass);
	gppi(sesskey, "SSHLogOmitData", 0, conf, CONF_logomitdata);

	prot = gpps_raw(sesskey, "Protocol", "default");
	conf_set_int(conf, CONF_protocol, default_protocol);
	conf_set_int(conf, CONF_port, default_port);
	{
		const Backend *b = backend_from_name(prot);
		if (b) {
			conf_set_int(conf, CONF_protocol, b->protocol);
			gppi(sesskey, "PortNumber", default_port, conf, CONF_port);
		}
	}
	sfree(prot);

	/* Address family selection */
	gppi(sesskey, "AddressFamily", ADDRTYPE_UNSPEC, conf, CONF_addressfamily);

	/* The CloseOnExit numbers are arranged in a different order from
	* the standard FORCE_ON / FORCE_OFF / AUTO. */
	i = gppi_raw(sesskey, "CloseOnExit", 1); conf_set_int(conf, CONF_close_on_exit, (i+1)%3);
	gppi(sesskey, "WarnOnClose", 1, conf, CONF_warn_on_close);
	{
		/* This is two values for backward compatibility with 0.50/0.51 */
		int pingmin, pingsec;
		pingmin = gppi_raw(sesskey, "PingInterval", 0);
		pingsec = gppi_raw(sesskey, "PingIntervalSecs", 0);
		conf_set_int(conf, CONF_ping_interval, pingmin * 60 + pingsec);
	}
	gppi(sesskey, "TCPNoDelay", 1, conf, CONF_tcp_nodelay);
	gppi(sesskey, "TCPKeepalives", 0, conf, CONF_tcp_keepalives);
	gpps(sesskey, "TerminalType", "xterm", conf, CONF_termtype);
	gpps(sesskey, "TerminalSpeed", "38400,38400", conf, CONF_termspeed);
	if (!gppmap(sesskey, "TerminalModes", conf, CONF_ttymodes)) {
		/* This hardcodes a big set of defaults in any new saved
		* sessions. Let's hope we don't change our mind. */
		for (i = 0; ttymodes[i]; i++)
			conf_set_str_str(conf, CONF_ttymodes, ttymodes[i], "A");
	}

	/* proxy settings */
	gpps(sesskey, "ProxyExcludeList", "", conf, CONF_proxy_exclude_list);
	i = gppi_raw(sesskey, "ProxyDNS", 1); conf_set_int(conf, CONF_proxy_dns, (i+1)%3);
	gppi(sesskey, "ProxyLocalhost", 0, conf, CONF_even_proxy_localhost);
	gppi(sesskey, "ProxyMethod", -1, conf, CONF_proxy_type);
	if (conf_get_int(conf, CONF_proxy_type) == -1) {
		int i;
		i = gppi_raw(sesskey, "ProxyType", 0);
		if (i == 0)
			conf_set_int(conf, CONF_proxy_type, PROXY_NONE);
		else if (i == 1)
			conf_set_int(conf, CONF_proxy_type, PROXY_HTTP);
		else if (i == 3)
			conf_set_int(conf, CONF_proxy_type, PROXY_TELNET);
		else if (i == 4)
			conf_set_int(conf, CONF_proxy_type, PROXY_CMD);
		else {
			i = gppi_raw(sesskey, "ProxySOCKSVersion", 5);
			if (i == 5)
				conf_set_int(conf, CONF_proxy_type, PROXY_SOCKS5);
			else
				conf_set_int(conf, CONF_proxy_type, PROXY_SOCKS4);
		}
	}
	gpps(sesskey, "ProxyHost", "proxy", conf, CONF_proxy_host);
	gppi(sesskey, "ProxyPort", 80, conf, CONF_proxy_port);
	gpps(sesskey, "ProxyUsername", "", conf, CONF_proxy_username);
	gpps(sesskey, "ProxyPassword", "", conf, CONF_proxy_password);
	gpps(sesskey, "ProxyTelnetCommand", "connect %host %port\\n",
		conf, CONF_proxy_telnet_command);
	gppmap(sesskey, "Environment", conf, CONF_environmt);
	gpps(sesskey, "UserName", "", conf, CONF_username);
	gppi(sesskey, "UserNameFromEnvironment", 0, conf, CONF_username_from_env);
	gpps(sesskey, "LocalUserName", "", conf, CONF_localusername);
	gppi(sesskey, "NoPTY", 0, conf, CONF_nopty);
	gppi(sesskey, "Compression", 0, conf, CONF_compression);
	gppi(sesskey, "TryAgent", 1, conf, CONF_tryagent);
	gppi(sesskey, "AgentFwd", 0, conf, CONF_agentfwd);
	gppi(sesskey, "ChangeUsername", 0, conf, CONF_change_username);
	gppi(sesskey, "GssapiFwd", 0, conf, CONF_gssapifwd);
	gprefs(sesskey, "Cipher", "\0",
		ciphernames, CIPHER_MAX, conf, CONF_ssh_cipherlist);
	{
		/* Backward-compatibility: we used to have an option to
		* disable gex under the "bugs" panel after one report of
		* a server which offered it then choked, but we never got
		* a server version string or any other reports. */
		char *default_kexes;
		i = 2 - gppi_raw(sesskey, "BugDHGEx2", 0);
		if (i == FORCE_ON)
			default_kexes = "dh-group14-sha1,dh-group1-sha1,rsa,WARN,dh-gex-sha1";
		else
			default_kexes = "dh-gex-sha1,dh-group14-sha1,dh-group1-sha1,rsa,WARN";
		gprefs(sesskey, "KEX", default_kexes,
			kexnames, KEX_MAX, conf, CONF_ssh_kexlist);
	}
	gppi(sesskey, "RekeyTime", 60, conf, CONF_ssh_rekey_time);
	gpps(sesskey, "RekeyBytes", "1G", conf, CONF_ssh_rekey_data);
	gppi(sesskey, "SshProt", 2, conf, CONF_sshprot);
	gpps(sesskey, "LogHost", "", conf, CONF_loghost);
	gppi(sesskey, "SSH2DES", 0, conf, CONF_ssh2_des_cbc);
	gppi(sesskey, "SshNoAuth", 0, conf, CONF_ssh_no_userauth);
	gppi(sesskey, "SshBanner", 1, conf, CONF_ssh_show_banner);
	gppi(sesskey, "AuthTIS", 0, conf, CONF_try_tis_auth);
	gppi(sesskey, "AuthKI", 1, conf, CONF_try_ki_auth);
	gppi(sesskey, "AuthGSSAPI", 1, conf, CONF_try_gssapi_auth);
#ifndef NO_GSSAPI
	gprefs(sesskey, "GSSLibs", "\0",
		gsslibkeywords, ngsslibs, conf, CONF_ssh_gsslist);
	gppfile(sesskey, "GSSCustom", conf, CONF_ssh_gss_custom);
#endif
	gppi(sesskey, "SshNoShell", 0, conf, CONF_ssh_no_shell);
	gppfile(sesskey, "PublicKeyFile", conf, CONF_keyfile);
	gpps(sesskey, "RemoteCommand", "", conf, CONF_remote_cmd);
	gppi(sesskey, "RFCEnviron", 0, conf, CONF_rfc_environ);
	gppi(sesskey, "PassiveTelnet", 0, conf, CONF_passive_telnet);
	gppi(sesskey, "BackspaceIsDelete", 1, conf, CONF_bksp_is_delete);
	gppi(sesskey, "RXVTHomeEnd", 0, conf, CONF_rxvt_homeend);
	gppi(sesskey, "LinuxFunctionKeys", 0, conf, CONF_funky_type);
	gppi(sesskey, "NoApplicationKeys", 0, conf, CONF_no_applic_k);
	gppi(sesskey, "NoApplicationCursors", 0, conf, CONF_no_applic_c);
	gppi(sesskey, "NoMouseReporting", 0, conf, CONF_no_mouse_rep);
	gppi(sesskey, "NoRemoteResize", 0, conf, CONF_no_remote_resize);
	gppi(sesskey, "NoAltScreen", 0, conf, CONF_no_alt_screen);
	gppi(sesskey, "NoRemoteWinTitle", 0, conf, CONF_no_remote_wintitle);
	{
		/* Backward compatibility */
		int no_remote_qtitle = gppi_raw(sesskey, "NoRemoteQTitle", 1);
		/* We deliberately interpret the old setting of "no response" as
		* "empty string". This changes the behaviour, but hopefully for
		* the better; the user can always recover the old behaviour. */
		gppi(sesskey, "RemoteQTitleAction",
			no_remote_qtitle ? TITLE_EMPTY : TITLE_REAL,
			conf, CONF_remote_qtitle_action);
	}
	gppi(sesskey, "NoDBackspace", 0, conf, CONF_no_dbackspace);
	gppi(sesskey, "NoRemoteCharset", 0, conf, CONF_no_remote_charset);
	gppi(sesskey, "ApplicationCursorKeys", 0, conf, CONF_app_cursor);
	gppi(sesskey, "ApplicationKeypad", 0, conf, CONF_app_keypad);
	gppi(sesskey, "NetHackKeypad", 0, conf, CONF_nethack_keypad);
	gppi(sesskey, "AltF4", 1, conf, CONF_alt_f4);
	gppi(sesskey, "AltSpace", 0, conf, CONF_alt_space);
	gppi(sesskey, "AltOnly", 0, conf, CONF_alt_only);
	gppi(sesskey, "ComposeKey", 0, conf, CONF_compose_key);
	gppi(sesskey, "CtrlAltKeys", 1, conf, CONF_ctrlaltkeys);
	gppi(sesskey, "TelnetKey", 0, conf, CONF_telnet_keyboard);
	gppi(sesskey, "TelnetRet", 1, conf, CONF_telnet_newline);
	gppi(sesskey, "LocalEcho", AUTO, conf, CONF_localecho);
	gppi(sesskey, "LocalEdit", AUTO, conf, CONF_localedit);
	gpps(sesskey, "Answerback", "PuTTY", conf, CONF_answerback);
	gppi(sesskey, "AlwaysOnTop", 0, conf, CONF_alwaysontop);
	gppi(sesskey, "FullScreenOnAltEnter", 0, conf, CONF_fullscreenonaltenter);
	gppi(sesskey, "HideMousePtr", 0, conf, CONF_hide_mouseptr);
	gppi(sesskey, "SunkenEdge", 0, conf, CONF_sunken_edge);
	gppi(sesskey, "WindowBorder", 1, conf, CONF_window_border);
	gppi(sesskey, "CurType", 0, conf, CONF_cursor_type);
	gppi(sesskey, "BlinkCur", 0, conf, CONF_blink_cur);
	/* pedantic compiler tells me I can't use conf, CONF_beep as an int * :-) */
	gppi(sesskey, "Beep", 1, conf, CONF_beep);
	gppi(sesskey, "BeepInd", 0, conf, CONF_beep_ind);
	gppfile(sesskey, "BellWaveFile", conf, CONF_bell_wavefile);
	gppi(sesskey, "BellOverload", 1, conf, CONF_bellovl);
	gppi(sesskey, "BellOverloadN", 5, conf, CONF_bellovl_n);
	i = gppi_raw(sesskey, "BellOverloadT", 2*TICKSPERSEC
#ifdef PUTTY_UNIX_H
		*1000
#endif
		);
	conf_set_int(conf, CONF_bellovl_t, i
#ifdef PUTTY_UNIX_H
		/ 1000
#endif
		);
	i = gppi_raw(sesskey, "BellOverloadS", 5*TICKSPERSEC
#ifdef PUTTY_UNIX_H
		*1000
#endif
		);
	conf_set_int(conf, CONF_bellovl_s, i
#ifdef PUTTY_UNIX_H
		/ 1000
#endif
		);
	gppi(sesskey, "ScrollbackLines", 200, conf, CONF_savelines);
	gppi(sesskey, "DECOriginMode", 0, conf, CONF_dec_om);
	gppi(sesskey, "AutoWrapMode", 1, conf, CONF_wrap_mode);
	gppi(sesskey, "LFImpliesCR", 0, conf, CONF_lfhascr);
	gppi(sesskey, "CRImpliesLF", 0, conf, CONF_crhaslf);
	gppi(sesskey, "DisableArabicShaping", 0, conf, CONF_arabicshaping);
	gppi(sesskey, "DisableBidi", 0, conf, CONF_bidi);
	gppi(sesskey, "WinNameAlways", 1, conf, CONF_win_name_always);
	gpps(sesskey, "WinTitle", "", conf, CONF_wintitle);
	gppi(sesskey, "TermWidth", 80, conf, CONF_width);
	gppi(sesskey, "TermHeight", 24, conf, CONF_height);
	gppfont(sesskey, "Font", conf, CONF_font);
	gppi(sesskey, "FontQuality", FQ_DEFAULT, conf, CONF_font_quality);
	gppi(sesskey, "FontVTMode", VT_UNICODE, conf, CONF_vtmode);
	gppi(sesskey, "UseSystemColours", 0, conf, CONF_system_colour);
	gppi(sesskey, "TryPalette", 0, conf, CONF_try_palette);
	gppi(sesskey, "ANSIColour", 1, conf, CONF_ansi_colour);
	gppi(sesskey, "Xterm256Colour", 1, conf, CONF_xterm_256_colour);
	i = gppi_raw(sesskey, "BoldAsColour", 0); conf_set_int(conf, CONF_bold_style, i+1);

	for (i = 0; i < 22; i++) {
		static const char *const defaults[] = {
			"187,187,187", "255,255,255", "0,0,0", "85,85,85", "0,0,0",
			"0,255,0", "0,0,0", "85,85,85", "187,0,0", "255,85,85",
			"0,187,0", "85,255,85", "187,187,0", "255,255,85", "0,0,187",
			"85,85,255", "187,0,187", "255,85,255", "0,187,187",
			"85,255,255", "187,187,187", "255,255,255"
		};
		char buf[20], *buf2;
		int c0, c1, c2;
		sprintf(buf, "Colour%d", i);
		buf2 = gpps_raw(sesskey, buf, defaults[i]);
		if (sscanf(buf2, "%d,%d,%d", &c0, &c1, &c2) == 3) {
			conf_set_int_int(conf, CONF_colours, i*3+0, c0);
			conf_set_int_int(conf, CONF_colours, i*3+1, c1);
			conf_set_int_int(conf, CONF_colours, i*3+2, c2);
		}
		sfree(buf2);
	}
	gppi(sesskey, "RawCNP", 0, conf, CONF_rawcnp);
	gppi(sesskey, "PasteRTF", 0, conf, CONF_rtf_paste);
	gppi(sesskey, "MouseIsXterm", 0, conf, CONF_mouse_is_xterm);
	gppi(sesskey, "RectSelect", 0, conf, CONF_rect_select);
	gppi(sesskey, "MouseOverride", 1, conf, CONF_mouse_override);
	for (i = 0; i < 256; i += 32) {
		static const char *const defaults[] = {
			"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
			"0,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1",
			"1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,2",
			"1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1",
			"1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1",
			"1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1",
			"2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2",
			"2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2"
		};
		char buf[20], *buf2, *p;
		int j;
		sprintf(buf, "Wordness%d", i);
		buf2 = gpps_raw(sesskey, buf, defaults[i / 32]);
		p = buf2;
		for (j = i; j < i + 32; j++) {
			char *q = p;
			while (*p && *p != ',')
				p++;
			if (*p == ',')
				*p++ = '\0';
			conf_set_int_int(conf, CONF_wordness, j, atoi(q));
		}
		sfree(buf2);
	}
	/*
	* The empty default for LineCodePage will be converted later
	* into a plausible default for the locale.
	*/
	gpps(sesskey, "LineCodePage", "", conf, CONF_line_codepage);
	gppi(sesskey, "CJKAmbigWide", 0, conf, CONF_cjk_ambig_wide);
	gppi(sesskey, "UTF8Override", 1, conf, CONF_utf8_override);
	gpps(sesskey, "Printer", "", conf, CONF_printer);
	gppi(sesskey, "CapsLockCyr", 0, conf, CONF_xlat_capslockcyr);
	gppi(sesskey, "ScrollBar", 1, conf, CONF_scrollbar);
	gppi(sesskey, "ScrollBarFullScreen", 0, conf, CONF_scrollbar_in_fullscreen);
	gppi(sesskey, "ScrollOnKey", 0, conf, CONF_scroll_on_key);
	gppi(sesskey, "ScrollOnDisp", 1, conf, CONF_scroll_on_disp);
	gppi(sesskey, "EraseToScrollback", 1, conf, CONF_erase_to_scrollback);
	gppi(sesskey, "LockSize", 0, conf, CONF_resize_action);
	gppi(sesskey, "BCE", 1, conf, CONF_bce);
	gppi(sesskey, "BlinkText", 0, conf, CONF_blinktext);
	gppi(sesskey, "X11Forward", 0, conf, CONF_x11_forward);
	gpps(sesskey, "X11Display", "", conf, CONF_x11_display);
	gppi(sesskey, "X11AuthType", X11_MIT, conf, CONF_x11_auth);
	gppfile(sesskey, "X11AuthFile", conf, CONF_xauthfile);

	gppi(sesskey, "LocalPortAcceptAll", 0, conf, CONF_lport_acceptall);
	gppi(sesskey, "RemotePortAcceptAll", 0, conf, CONF_rport_acceptall);
	gppmap(sesskey, "PortForwardings", conf, CONF_portfwd);
	i = gppi_raw(sesskey, "BugIgnore1", 0); conf_set_int(conf, CONF_sshbug_ignore1, 2-i);
	i = gppi_raw(sesskey, "BugPlainPW1", 0); conf_set_int(conf, CONF_sshbug_plainpw1, 2-i);
	i = gppi_raw(sesskey, "BugRSA1", 0); conf_set_int(conf, CONF_sshbug_rsa1, 2-i);
	i = gppi_raw(sesskey, "BugIgnore2", 0); conf_set_int(conf, CONF_sshbug_ignore2, 2-i);
	{
		int i;
		i = gppi_raw(sesskey, "BugHMAC2", 0); conf_set_int(conf, CONF_sshbug_hmac2, 2-i);
		if (2-i == AUTO) {
			i = gppi_raw(sesskey, "BuggyMAC", 0);
			if (i == 1)
				conf_set_int(conf, CONF_sshbug_hmac2, FORCE_ON);
		}
	}
	i = gppi_raw(sesskey, "BugDeriveKey2", 0); conf_set_int(conf, CONF_sshbug_derivekey2, 2-i);
	i = gppi_raw(sesskey, "BugRSAPad2", 0); conf_set_int(conf, CONF_sshbug_rsapad2, 2-i);
	i = gppi_raw(sesskey, "BugPKSessID2", 0); conf_set_int(conf, CONF_sshbug_pksessid2, 2-i);
	i = gppi_raw(sesskey, "BugRekey2", 0); conf_set_int(conf, CONF_sshbug_rekey2, 2-i);
	i = gppi_raw(sesskey, "BugMaxPkt2", 0); conf_set_int(conf, CONF_sshbug_maxpkt2, 2-i);
	i = gppi_raw(sesskey, "BugWinadj", 0); conf_set_int(conf, CONF_sshbug_winadj, 2-i);
	conf_set_int(conf, CONF_ssh_simple, FALSE);
	gppi(sesskey, "StampUtmp", 1, conf, CONF_stamp_utmp);
	gppi(sesskey, "LoginShell", 1, conf, CONF_login_shell);
	gppi(sesskey, "ScrollbarOnLeft", 0, conf, CONF_scrollbar_on_left);
	gppi(sesskey, "ShadowBold", 0, conf, CONF_shadowbold);
	gppfont(sesskey, "BoldFont", conf, CONF_boldfont);
	gppfont(sesskey, "WideFont", conf, CONF_widefont);
	gppfont(sesskey, "WideBoldFont", conf, CONF_wideboldfont);
	gppi(sesskey, "ShadowBoldOffset", 1, conf, CONF_shadowboldoffset);
	gpps(sesskey, "SerialLine", "", conf, CONF_serline);
	gppi(sesskey, "SerialSpeed", 9600, conf, CONF_serspeed);
	gppi(sesskey, "SerialDataBits", 8, conf, CONF_serdatabits);
	gppi(sesskey, "SerialStopHalfbits", 2, conf, CONF_serstopbits);
	gppi(sesskey, "SerialParity", SER_PAR_NONE, conf, CONF_serparity);
	gppi(sesskey, "SerialFlowControl", SER_FLOW_XONXOFF, conf, CONF_serflow);
	gpps(sesskey, "WindowClass", "", conf, CONF_winclass);

	// SFTP4TC
	gppi(sesskey, "SFTP4TC:cacheFolders", 0, conf, CONF_sftpCacheFolders);
	gppi(sesskey, "SFTP4TC:hideDotNames", 0, conf, CONF_sftpHideDotNames);
	gpps(sesskey, "SFTP4TC:defChMod", "", conf, CONF_sftpDefChMod);
	gpps(sesskey, "SFTP4TC:exeChMod", "", conf, CONF_sftpExeChMod);
	gpps(sesskey, "SFTP4TC:exeExtensions", "", conf, CONF_sftpExeExtensions);
	gpps(sesskey, "SFTP4TC:homeDir", "", conf, CONF_sftpHomeDir);
	gpps(sesskey, "SFTP4TC:sftpCommand", "", conf, CONF_sftpCommand);
	gppi(sesskey, "SFTP4TC:storePassword", 0, conf, CONF_sftpStorePassword);

}

void do_defaults(char *session, struct Sftp4tc *conf)
{
	load_settings(session, conf);
}

static int sessioncmp(const void *av, const void *bv)
{
	const char *a = *(const char *const *) av;
	const char *b = *(const char *const *) bv;

	/*
	* Alphabetical order, except that "Default Settings" is a
	* special case and comes first.
	*/
	if (!strcmp(a, "Default Settings"))
		return -1;		       /* a comes first */
	if (!strcmp(b, "Default Settings"))
		return +1;		       /* b comes first */
	/*
	* FIXME: perhaps we should ignore the first & in determining
	* sort order.
	*/
	return strcmp(a, b);	       /* otherwise, compare normally */
}

void get_sesslist(struct sesslist *list, int allocate)
{
	char otherbuf[2048];
	int buflen, bufsize, i;
	char *p, *ret;
	void *handle;

	if (allocate) {

		buflen = bufsize = 0;
		list->buffer = NULL;
		if ((handle = enum_settings_start()) != NULL) {
			do {
				ret = enum_settings_next(handle, otherbuf, sizeof(otherbuf));
				if (ret) {
					int len = (int)strlen(otherbuf) + 1;
					if (bufsize < buflen + len) {
						bufsize = buflen + len + 2048;
						list->buffer = sresize(list->buffer, bufsize, char);
					}
					strcpy(list->buffer + buflen, otherbuf);
					buflen += (int)strlen(list->buffer + buflen) + 1;
				}
			} while (ret);
			enum_settings_finish(handle);
		}
		list->buffer = sresize(list->buffer, buflen + 1, char);
		list->buffer[buflen] = '\0';

		/*
		* Now set up the list of sessions. Note that "Default
		* Settings" must always be claimed to exist, even if it
		* doesn't really.
		*/

		p = list->buffer;
		list->nsessions = 1;	       /* "Default Settings" counts as one */
		while (*p) {
			if (strcmp(p, "Default Settings"))
				list->nsessions++;
			while (*p)
				p++;
			p++;
		}

		list->sessions = snewn(list->nsessions + 1, char *);
		list->sessions[0] = "Default Settings";
		p = list->buffer;
		i = 1;
		while (*p) {
			if (strcmp(p, "Default Settings"))
				list->sessions[i++] = p;
			while (*p)
				p++;
			p++;
		}

		qsort(list->sessions, i, sizeof(char *), sessioncmp);
	} else {
		sfree(list->buffer);
		sfree(list->sessions);
		list->buffer = NULL;
		list->sessions = NULL;
	}
}
