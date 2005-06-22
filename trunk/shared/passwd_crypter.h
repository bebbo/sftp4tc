#ifndef _PASSWD_CRYPTER
#define _PASSWD_CRYPTER

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*tSetupPasswordCrypter) (char *Passphrase);
typedef int (*tEncryptPassword) (char *PlainPassword, char *EncryptedPassword);
typedef int (*tDecryptPassword) (char *EncryptedPassword, char *PlainPassword);

#ifdef __cplusplus
}
#endif

#endif //_PASSWD_CRYPTER
