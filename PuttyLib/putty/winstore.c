/*
* winstore.c: Windows-specific implementation of the interface
* defined in storage.h.
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "putty.h"
#include "storage.h"

#include <shlobj.h>
#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA 0x001a
#endif
#ifndef CSIDL_LOCAL_APPDATA
#define CSIDL_LOCAL_APPDATA 0x001c
#endif

/**
 * to keep track of a registry entry or an ini section.
 */
struct KeyOrIni {
	void * key;
	char * ini;
	char * section;
};

/**
 * These ini paths define the search order for ini files.
 * Registry is always searched first.
 * newer found locations overwrite older locations (%home% wins always).
 * The patterns are expanded to real paths.
 */
static char * iniPaths[] = {
	"%windows%",
	"%app%",
	"%home%",
	0
};

static const char *const puttystr = PUTTY_REG_POS "\\Sessions";

static const char hex[16] = "0123456789ABCDEF";

static int tried_shgetfolderpath = FALSE;
static HMODULE shell32_module = NULL;
typedef HRESULT (WINAPI *p_SHGetFolderPath_t)
(HWND, int, HANDLE, DWORD, LPTSTR);
static p_SHGetFolderPath_t p_SHGetFolderPath = NULL;

static void mungestr(const char *in, char *out)
{
	int candot = 0;

	while (*in) {
		if (*in == ' ' || *in == '\\' || *in == '*' || *in == '?' ||
			*in == '%' || *in < ' ' || *in > '~' || (*in == '.'
			&& !candot)) {
				*out++ = '%';
				*out++ = hex[((unsigned char) *in) >> 4];
				*out++ = hex[((unsigned char) *in) & 15];
		} else
			*out++ = *in;
		in++;
		candot = 1;
	}
	*out = '\0';
	return;
}

static void unmungestr(const char *in, char *out, int outlen)
{
	while (*in) {
		if (*in == '%' && in[1] && in[2]) {
			int i, j;

			i = in[1] - '0';
			i -= (i > 9 ? 7 : 0);
			j = in[2] - '0';
			j -= (j > 9 ? 7 : 0);

			*out++ = (i << 4) + j;
			if (!--outlen)
				return;
			in += 3;
		} else {
			*out++ = *in++;
			if (!--outlen)
				return;
		}
	}
	*out = '\0';
	return;
}

void *open_settings_w(const char *sessionname, Config * cfg, char **errmsg)
{
	HKEY subkey1;
	int ret;
	char *p;
	struct KeyOrIni * keyOrIni;

	keyOrIni = (struct KeyOrIni *)smalloc(sizeof(struct KeyOrIni));
	if (!keyOrIni)
		return 0;

	keyOrIni->key = 0;
	keyOrIni->ini = cfg->iniPath;

	*errmsg = NULL;

	if (!sessionname || !*sessionname)
		sessionname = "Default Settings";

	keyOrIni->section = p = snewn(3 * strlen(sessionname) + 1, char);
	mungestr(sessionname, p);

	if (keyOrIni->ini[0] == 0) {
  	  ret = RegCreateKey(HKEY_CURRENT_USER, puttystr, &subkey1);
	  if (ret != ERROR_SUCCESS) {
		*errmsg = dupprintf("Unable to create registry key\n"
			"HKEY_CURRENT_USER\\%s", puttystr);
		close_settings_w(keyOrIni);
		return NULL;
	  }
	  ret = RegCreateKey(subkey1, p, &(HKEY)keyOrIni->key);
	  RegCloseKey(subkey1);
	  if (ret != ERROR_SUCCESS) {
		*errmsg = dupprintf("Unable to create registry key\n"
			"HKEY_CURRENT_USER\\%s\\%s", puttystr, p);
		return NULL;
	  }
	}
	return keyOrIni;
}

int write_setting_s(struct KeyOrIni *handle, const char *key, const char *value)
{
	if (!handle)
		return 0;

	if (handle->key) {
		return ERROR_SUCCESS == RegSetValueEx((HKEY) handle->key, key, 0, REG_SZ, value, 1 + strlen(value));
	}
	
	// whoops
	if (!handle->ini || !handle->section)
		return 0;

	return WritePrivateProfileString(handle->section, key, value, handle->ini) > 0;
}

int write_setting_i(struct KeyOrIni *handle, const char *key, int value)
{
	if (!handle)
		return 0;

	if (handle->key) {
		return ERROR_SUCCESS == RegSetValueEx((HKEY) handle->key, key, 0, REG_DWORD,
		    (CONST BYTE *) &value, sizeof(value));
	}
	// whoops
	if (!handle->ini || !handle->section)
		return 0;

	{
		char txt[32];
		sprintf(txt, "%d", value);
		return WritePrivateProfileString(handle->section, key, txt, handle->ini) > 0;
	}
}

void close_settings_w(struct KeyOrIni *handle)
{
	if (!handle)
		return;

	if (handle->key)
		RegCloseKey((HKEY) handle->key);
	if (handle->section)
		sfree(handle->section);

	sfree(handle);
}

char * get_initpath_by_index(int index) {
	char * path;
	if (index >= sizeof(iniPaths)/sizeof(iniPaths[0]))
		return 0;

	path = iniPaths[index];
	if (!path) return 0;

	if (*path != '%') return path;

	// expand the path

	if (0 == strcmp("%app%", path)) {
		static char appPath[1024];
		int sz = GetModuleFileName(0, appPath, 1024);
		if (sz > 1012)
			return 0;
		// remove the file name
		while (sz > 0 && appPath[sz-1] != '\\') {
			--sz;
			appPath[sz] = 0;
		}
		if (sz > 0 && appPath[sz-1] == '\\')
			appPath[sz - 1] = 0;
		path = appPath;
	} else
	if (0 == strcmp("%windows%", path)) {
		static char windowsPath[1024];
		int sz = GetWindowsDirectory(windowsPath, 1024);
		if (sz > 1012)
			return 0;
		path = windowsPath;
	} else
	if (0 == strcmp("%home%", path)) {
		static char homePath[1024];
		int sz = GetEnvironmentVariable("HOMEDRIVE", homePath, 1024);
		sz += GetEnvironmentVariable("HOMEPATH", homePath + sz, 1024 - sz);
		if (sz > 1012)
			return 0;
		path = homePath;
	} else {
		return 0;
	}

	strcat(path, "\\putty.ini");
	iniPaths[index] = path;
	return path;
}

/**
 * Get a handle for a session name.
 * Also update the cfg with the ini path, if an ini is used.
 */
void * open_settings_r(const char *sessionname, Config * cfg)
{
	HKEY subkey1;
	char *p;
	struct KeyOrIni * keyOrIni;

	// kill last ini path
	cfg->iniPath[0] = 0;

	keyOrIni = (struct KeyOrIni *)smalloc(sizeof(struct KeyOrIni));
	if (!keyOrIni)
		return 0;

	keyOrIni->key = 0;
	keyOrIni->ini = 0;

	if (!sessionname || !*sessionname)
		sessionname = "Default Settings";

	keyOrIni->section =	p = snewn(3 * strlen(sessionname) + 1, char);
	mungestr(sessionname, p);

	// search registry first
	if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &subkey1) != ERROR_SUCCESS) {
		keyOrIni->key = NULL;
	} else {
		if (RegOpenKey(subkey1, p, &(HKEY)keyOrIni->key) != ERROR_SUCCESS) {
			keyOrIni->key = NULL;
		}
		RegCloseKey(subkey1);
	}

	// now search all ini files
	{
		int i, len;
		char buff[256];
		for (i = 0; i < sizeof(iniPaths)/sizeof(iniPaths[0]); ++i) {
			p = get_initpath_by_index(i);

			// skip invalid paths
			if (!p)
				continue;

			len = GetPrivateProfileString(keyOrIni->section, 0, 0, buff, 256, p);
			if (len > 0) {
				if (keyOrIni->key) {
					RegCloseKey((HKEY)keyOrIni->key);
					keyOrIni->key = 0;
				}
				keyOrIni->ini = p;
			}
		}
	}
	
	// update cfg
	if (keyOrIni->ini)
		strcpy(cfg->iniPath, keyOrIni->ini);

	return keyOrIni;
}

/**
 * Read the string value either from registry or from the specified section inside the ini file.
 */
char *read_setting_s(struct KeyOrIni *handle, const char *key, char *buffer, int buflen)
{
	DWORD type, size;
	size = buflen;

	if (!handle) return 0;

	// use the registry key if present
	if (handle->key) {
		if (RegQueryValueEx((HKEY) handle->key, key, 0, &type, buffer, &size) != ERROR_SUCCESS ||
		        type != REG_SZ)
			return NULL;
		return buffer;
	}

	// whoops - nothing set
	if (!handle->ini || !handle->section)
		return 0;

	// use the specified ini
	size = GetPrivateProfileString(handle->section, key, 0, buffer, buflen, handle->ini);
	if (size == 0 || size == buflen - 1)
		return 0;
	return buffer;
}

/**
 * Read the int value either from registry or from the specified section inside the ini file.
 */
int read_setting_i(struct KeyOrIni *handle, const char *key, int defvalue)
{
	DWORD type, val, size;
	size = sizeof(val);

	if (!handle) return defvalue;

	if (handle->key) {
		if (RegQueryValueEx((HKEY) handle->key, key, 0, &type, (BYTE *) &val, &size) != ERROR_SUCCESS ||
		        size != sizeof(val) || type != REG_DWORD)
		    return defvalue;
		return val;
	}

	// whoops - nothing set
	if (!handle->ini || !handle->section)
		return defvalue;

	// use the specified ini
	return GetPrivateProfileInt(handle->section, key, defvalue, handle->ini);
}

int read_setting_fontspec(struct KeyOrIni *handle, const char *name, FontSpec *result)
{
	char *settingname;
	FontSpec ret;

	if (!read_setting_s(handle, name, ret.name, sizeof(ret.name)))
		return 0;
	settingname = dupcat(name, "IsBold", NULL);
	ret.isbold = read_setting_i(handle, settingname, -1);
	sfree(settingname);
	if (ret.isbold == -1) return 0;
	settingname = dupcat(name, "CharSet", NULL);
	ret.charset = read_setting_i(handle, settingname, -1);
	sfree(settingname);
	if (ret.charset == -1) return 0;
	settingname = dupcat(name, "Height", NULL);
	ret.height = read_setting_i(handle, settingname, INT_MIN);
	sfree(settingname);
	if (ret.height == INT_MIN) return 0;
	*result = ret;
	return 1;
}

int write_setting_fontspec(struct KeyOrIni *handle, const char *name, FontSpec font)
{
	int ret;
	char *settingname;

	ret = write_setting_s(handle, name, font.name);
	settingname = dupcat(name, "IsBold", NULL);
	ret &= write_setting_i(handle, settingname, font.isbold);
	sfree(settingname);
	settingname = dupcat(name, "CharSet", NULL);
	ret &= write_setting_i(handle, settingname, font.charset);
	sfree(settingname);
	settingname = dupcat(name, "Height", NULL);
	ret &= write_setting_i(handle, settingname, font.height);
	sfree(settingname);

	return ret;
}

int read_setting_filename(struct KeyOrIni *handle, const char *name, Filename *result)
{
	return !!read_setting_s(handle, name, result->path, sizeof(result->path));
}

int write_setting_filename(struct KeyOrIni *handle, const char *name, Filename result)
{
	return write_setting_s(handle, name, result.path);
}

void close_settings_r(struct KeyOrIni *handle)
{
	close_settings_w(handle);
}

int del_settings(const char *sessionname, Config * cfg)
{
	int ret = 0;
 	char *p = snewn(3 * strlen(sessionname) + 1, char);
	mungestr(sessionname, p);
	if (cfg->iniPath[0] == 0) {
		// delete from registry
		HKEY subkey1;
		if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &subkey1) == ERROR_SUCCESS) {
			ret = RegDeleteKey(subkey1, p) == ERROR_SUCCESS;
			RegCloseKey(subkey1);
		}
	} else {
		// or from ini file
		ret = WritePrivateProfileString(p, 0, 0, cfg->iniPath) > 0;
	}

	sfree(p);
	return ret;
}

struct sList {
	struct sList * next;
	char * string;
};

struct enumsettings {
	HKEY key;
	int i;
	char * iniPath;
	struct sList * first;
};

void *enum_settings_start(void)
{
	struct enumsettings *ret;
	HKEY key;

	if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &key) != ERROR_SUCCESS)
		return NULL;

	ret = snew(struct enumsettings);
	if (ret) {
		ret->key = key;
		ret->i = 0;
		ret->iniPath = 0;
		ret->first = 0;
	}

	return ret;
}

char *enum_settings_next(void *handle, char *buffer, int buflen)
{
	struct enumsettings *e = (struct enumsettings *) handle;
	struct sList * node;
	char *otherbuf;
	otherbuf = snewn(3 * buflen, char);

	// read registry
	if (!e->iniPath) {
		if (RegEnumKey(e->key, e->i, otherbuf, 3 * buflen) == ERROR_SUCCESS) {
			++e->i;
			unmungestr(otherbuf, buffer, buflen);

			node = snew(struct sList);
			node->next = e->first;
			e->first = node;
			node->string = strdup(buffer);

			sfree(otherbuf);
			return buffer;
		}		
	}
	sfree(otherbuf);

	// end of registry reached;
	if (e->key) {
		RegCloseKey(e->key);
		e->key = 0;
		e->i = 0;
	}

	while(e->i < sizeof(iniPaths)/sizeof(iniPaths[0])) {
		struct sList * n;
		int len;
		char * p, * sections;

		e->iniPath = get_initpath_by_index(e->i);
		if (!e->iniPath) {
			++e->i;
			continue;
		}

		// get all sections - and search the list to avoid duplicates
		len = 4096;
		sections = snewn(len, char);
		for(;;) {
			int read = GetPrivateProfileString(0, 0, 0, sections, len, e->iniPath);
			if (read < len - 1)
				break;
			sfree(sections);
			len += len;
			sections = snewn(len, char);
		}

		// search all sections
		p = sections;
		while(*p) {
			len = strlen(p);
			otherbuf = snewn(len + 1, char);
			unmungestr(p, otherbuf, len + 1);

			// does section exist?
			for (n = e->first; n ; n = n->next) {
				if (0 == strcmp(n->string, otherbuf))
					break;
			}
			// new section
			if (!n) {
				node = snew(struct sList);
				node->next = e->first;
				e->first = node;
				node->string = strdup(otherbuf);
				strncpy(buffer, otherbuf, buflen - 1);
				buffer[buflen - 1] = 0;

				sfree(otherbuf);
				return node->string;
			}
			sfree(otherbuf);
			p += len + 1;
		}
		++ e->i;
	}
	return 0;
}

void enum_settings_finish(void *handle)
{
	struct enumsettings *e = (struct enumsettings *) handle;
	struct sList * n;
	if (e->key)
		RegCloseKey(e->key);

	for (n = e->first; n;) {
		 struct sList * m = n->next;
		 sfree(n->string);
		 sfree(n);
		 n = m;
	}

	sfree(e);
}

static void hostkey_regname(char *buffer, const char *hostname,
							int port, const char *keytype)
{
	int len;
	strcpy(buffer, keytype);
	strcat(buffer, "@");
	len = strlen(buffer);
	len += sprintf(buffer + len, "%d:", port);
	mungestr(hostname, buffer + strlen(buffer));
}

int verify_host_key(const char *hostname, int port,
					const char *keytype, const char *key)
{
	char *otherstr, *regname;
	int len;
	HKEY rkey;
	DWORD readlen;
	DWORD type;
	int ret, compare;

	len = 1 + strlen(key);

	/*
	* Now read a saved key in from the registry and see what it
	* says.
	*/
	otherstr = snewn(len, char);
	regname = snewn(3 * (strlen(hostname) + strlen(keytype)) + 15, char);

	hostkey_regname(regname, hostname, port, keytype);

	if (RegOpenKey(HKEY_CURRENT_USER, PUTTY_REG_POS "\\SshHostKeys",
		&rkey) != ERROR_SUCCESS)
		return 1;		       /* key does not exist in registry */

	readlen = len;
	ret = RegQueryValueEx(rkey, regname, NULL, &type, otherstr, &readlen);

	if (ret != ERROR_SUCCESS && ret != ERROR_MORE_DATA &&
		!strcmp(keytype, "rsa")) {
			/*
			* Key didn't exist. If the key type is RSA, we'll try
			* another trick, which is to look up the _old_ key format
			* under just the hostname and translate that.
			*/
			char *justhost = regname + 1 + strcspn(regname, ":");
			char *oldstyle = snewn(len + 10, char);	/* safety margin */
			readlen = len;
			ret = RegQueryValueEx(rkey, justhost, NULL, &type,
				oldstyle, &readlen);

			if (ret == ERROR_SUCCESS && type == REG_SZ) {
				/*
				* The old format is two old-style bignums separated by
				* a slash. An old-style bignum is made of groups of
				* four hex digits: digits are ordered in sensible
				* (most to least significant) order within each group,
				* but groups are ordered in silly (least to most)
				* order within the bignum. The new format is two
				* ordinary C-format hex numbers (0xABCDEFG...XYZ, with
				* A nonzero except in the special case 0x0, which
				* doesn't appear anyway in RSA keys) separated by a
				* comma. All hex digits are lowercase in both formats.
				*/
				char *p = otherstr;
				char *q = oldstyle;
				int i, j;

				for (i = 0; i < 2; i++) {
					int ndigits, nwords;
					*p++ = '0';
					*p++ = 'x';
					ndigits = strcspn(q, "/");	/* find / or end of string */
					nwords = ndigits / 4;
					/* now trim ndigits to remove leading zeros */
					while (q[(ndigits - 1) ^ 3] == '0' && ndigits > 1)
						ndigits--;
					/* now move digits over to new string */
					for (j = 0; j < ndigits; j++)
						p[ndigits - 1 - j] = q[j ^ 3];
					p += ndigits;
					q += nwords * 4;
					if (*q) {
						q++;	       /* eat the slash */
						*p++ = ',';	       /* add a comma */
					}
					*p = '\0';	       /* terminate the string */
				}

				/*
				* Now _if_ this key matches, we'll enter it in the new
				* format. If not, we'll assume something odd went
				* wrong, and hyper-cautiously do nothing.
				*/
				if (!strcmp(otherstr, key))
					RegSetValueEx(rkey, regname, 0, REG_SZ, otherstr,
					strlen(otherstr) + 1);
			}
	}

	RegCloseKey(rkey);

	compare = strcmp(otherstr, key);

	sfree(otherstr);
	sfree(regname);

	if (ret == ERROR_MORE_DATA ||
		(ret == ERROR_SUCCESS && type == REG_SZ && compare))
		return 2;		       /* key is different in registry */
	else if (ret != ERROR_SUCCESS || type != REG_SZ)
		return 1;		       /* key does not exist in registry */
	else
		return 0;		       /* key matched OK in registry */
}

void store_host_key(const char *hostname, int port,
					const char *keytype, const char *key)
{
	char *regname;
	HKEY rkey;

	regname = snewn(3 * (strlen(hostname) + strlen(keytype)) + 15, char);

	hostkey_regname(regname, hostname, port, keytype);

	if (RegCreateKey(HKEY_CURRENT_USER, PUTTY_REG_POS "\\SshHostKeys",
		&rkey) == ERROR_SUCCESS) {
			RegSetValueEx(rkey, regname, 0, REG_SZ, key, strlen(key) + 1);
			RegCloseKey(rkey);
	} /* else key does not exist in registry */

	sfree(regname);
}

/*
* Open (or delete) the random seed file.
*/
enum { DEL, OPEN_R, OPEN_W };
static int try_random_seed(char const *path, int action, HANDLE *ret)
{
	if (action == DEL) {
		remove(path);
		*ret = INVALID_HANDLE_VALUE;
		return FALSE;		       /* so we'll do the next ones too */
	}

	*ret = CreateFile(path,
		action == OPEN_W ? GENERIC_WRITE : GENERIC_READ,
		action == OPEN_W ? 0 : (FILE_SHARE_READ |
		FILE_SHARE_WRITE),
		NULL,
		action == OPEN_W ? CREATE_ALWAYS : OPEN_EXISTING,
		action == OPEN_W ? FILE_ATTRIBUTE_NORMAL : 0,
		NULL);

	return (*ret != INVALID_HANDLE_VALUE);
}

static HANDLE access_random_seed(int action)
{
	HKEY rkey;
	DWORD type, size;
	HANDLE rethandle;
	char seedpath[2 * MAX_PATH + 10] = "\0";

	/*
	* Iterate over a selection of possible random seed paths until
	* we find one that works.
	* 
	* We do this iteration separately for reading and writing,
	* meaning that we will automatically migrate random seed files
	* if a better location becomes available (by reading from the
	* best location in which we actually find one, and then
	* writing to the best location in which we can _create_ one).
	*/

	/*
	* First, try the location specified by the user in the
	* Registry, if any.
	*/
	size = sizeof(seedpath);
	if (RegOpenKey(HKEY_CURRENT_USER, PUTTY_REG_POS, &rkey) ==
		ERROR_SUCCESS) {
			int ret = RegQueryValueEx(rkey, "RandSeedFile",
				0, &type, seedpath, &size);
			if (ret != ERROR_SUCCESS || type != REG_SZ)
				seedpath[0] = '\0';
			RegCloseKey(rkey);

			if (*seedpath && try_random_seed(seedpath, action, &rethandle))
				return rethandle;
	}

	/*
	* Next, try the user's local Application Data directory,
	* followed by their non-local one. This is found using the
	* SHGetFolderPath function, which won't be present on all
	* versions of Windows.
	*/
	if (!tried_shgetfolderpath) {
		/* This is likely only to bear fruit on systems with IE5+
		* installed, or WinMe/2K+. There is some faffing with
		* SHFOLDER.DLL we could do to try to find an equivalent
		* on older versions of Windows if we cared enough.
		* However, the invocation below requires IE5+ anyway,
		* so stuff that. */
		shell32_module = LoadLibrary("SHELL32.DLL");
		if (shell32_module) {
			p_SHGetFolderPath = (p_SHGetFolderPath_t)
				GetProcAddress(shell32_module, "SHGetFolderPathA");
		}
	}
	if (p_SHGetFolderPath) {
		if (SUCCEEDED(p_SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA,
			NULL, SHGFP_TYPE_CURRENT, seedpath))) {
				strcat(seedpath, "\\PUTTY.RND");
				if (try_random_seed(seedpath, action, &rethandle))
					return rethandle;
		}

		if (SUCCEEDED(p_SHGetFolderPath(NULL, CSIDL_APPDATA,
			NULL, SHGFP_TYPE_CURRENT, seedpath))) {
				strcat(seedpath, "\\PUTTY.RND");
				if (try_random_seed(seedpath, action, &rethandle))
					return rethandle;
		}
	}

	/*
	* Failing that, try %HOMEDRIVE%%HOMEPATH% as a guess at the
	* user's home directory.
	*/
	{
		int len, ret;

		len =
			GetEnvironmentVariable("HOMEDRIVE", seedpath,
			sizeof(seedpath));
		ret =
			GetEnvironmentVariable("HOMEPATH", seedpath + len,
			sizeof(seedpath) - len);
		if (ret != 0) {
			strcat(seedpath, "\\PUTTY.RND");
			if (try_random_seed(seedpath, action, &rethandle))
				return rethandle;
		}
	}

	/*
	* And finally, fall back to C:\WINDOWS.
	*/
	GetWindowsDirectory(seedpath, sizeof(seedpath));
	strcat(seedpath, "\\PUTTY.RND");
	if (try_random_seed(seedpath, action, &rethandle))
		return rethandle;

	/*
	* If even that failed, give up.
	*/
	return INVALID_HANDLE_VALUE;
}

void read_random_seed(noise_consumer_t consumer)
{
	HANDLE seedf = access_random_seed(OPEN_R);

	if (seedf != INVALID_HANDLE_VALUE) {
		while (1) {
			char buf[1024];
			DWORD len;

			if (ReadFile(seedf, buf, sizeof(buf), &len, NULL) && len)
				consumer(buf, len);
			else
				break;
		}
		CloseHandle(seedf);
	}
}

void write_random_seed(void *data, int len)
{
	HANDLE seedf = access_random_seed(OPEN_W);

	if (seedf != INVALID_HANDLE_VALUE) {
		DWORD lenwritten;

		WriteFile(seedf, data, len, &lenwritten, NULL);
		CloseHandle(seedf);
	}
}

/*
* Recursively delete a registry key and everything under it.
*/
static void registry_recursive_remove(HKEY key)
{
	DWORD i;
	char name[MAX_PATH + 1];
	HKEY subkey;

	i = 0;
	while (RegEnumKey(key, i, name, sizeof(name)) == ERROR_SUCCESS) {
		if (RegOpenKey(key, name, &subkey) == ERROR_SUCCESS) {
			registry_recursive_remove(subkey);
			RegCloseKey(subkey);
		}
		RegDeleteKey(key, name);
	}
}

void cleanup_all(void)
{
	HKEY key;
	int ret;
	char name[MAX_PATH + 1];

	/* ------------------------------------------------------------
	* Wipe out the random seed file, in all of its possible
	* locations.
	*/
	access_random_seed(DEL);

	/* ------------------------------------------------------------
	* Destroy all registry information associated with PuTTY.
	*/

	/*
	* Open the main PuTTY registry key and remove everything in it.
	*/
	if (RegOpenKey(HKEY_CURRENT_USER, PUTTY_REG_POS, &key) ==
		ERROR_SUCCESS) {
			registry_recursive_remove(key);
			RegCloseKey(key);
	}
	/*
	* Now open the parent key and remove the PuTTY main key. Once
	* we've done that, see if the parent key has any other
	* children.
	*/
	if (RegOpenKey(HKEY_CURRENT_USER, PUTTY_REG_PARENT,
		&key) == ERROR_SUCCESS) {
			RegDeleteKey(key, PUTTY_REG_PARENT_CHILD);
			ret = RegEnumKey(key, 0, name, sizeof(name));
			RegCloseKey(key);
			/*
			* If the parent key had no other children, we must delete
			* it in its turn. That means opening the _grandparent_
			* key.
			*/
			if (ret != ERROR_SUCCESS) {
				if (RegOpenKey(HKEY_CURRENT_USER, PUTTY_REG_GPARENT,
					&key) == ERROR_SUCCESS) {
						RegDeleteKey(key, PUTTY_REG_GPARENT_CHILD);
						RegCloseKey(key);
				}
			}
	}
	/*
	* Now we're done.
	*/
}
