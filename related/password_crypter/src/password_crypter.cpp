// password_container.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
//

#include "stdafx.h"
#include "openssl/blowfish.h"
#include "openssl/rand.h"
#include <openssl/bio.h> 
#include <openssl/evp.h> 

const int ERR_OK = 0;
const int ERR_KEY_LOAD_FAILED = 1;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
  return TRUE;
}

void EncodeBase64(char *in, char *out, int in_size)
{
	int i;
	BIO *mbio,*b64bio,*bio;
	char *p;

	mbio=BIO_new(BIO_s_mem());
	b64bio=BIO_new(BIO_f_base64());

	bio=BIO_push(b64bio, mbio);
	/* We now have bio pointing at b64->mem, the base64 bio encodes on
	 * write and decodes on read */

  BIO_write(bio, in, in_size);

  /* We need to 'flush' things to push out the encoding of the
	 * last few bytes.  There is special encoding if it is not a
	 * multiple of 3
	 */
	BIO_flush(bio);

	/* We will now get a pointer to the data and the number of elements. */
	/* hmm... this one was not defined by a macro in bio.h, it will be for
	 * 0.9.1.  The other option is too just read from the memory bio.
	 */
	i=(int)BIO_ctrl(mbio,BIO_CTRL_INFO,0,(char *)&p);

  memcpy(out, p, i);
  out[i-1]=0;

	/* This call will walk the chain freeing all the BIOs */
	BIO_free_all(bio);
}

int DecodeBase64(char *in, char *out, int in_size)
{
	int i;
	BIO *mbio,*b64bio,*bio;
  char new_line=10;

	mbio=BIO_new(BIO_s_mem());
	b64bio=BIO_new(BIO_f_base64());

	bio=BIO_push(b64bio, mbio);
	/* We now have bio pointing at b64->mem, the base64 bio encodes on
	 * write and decodes on read */

  BIO_write(mbio, in, in_size);
  BIO_write(mbio, &new_line, 1);

  /* We need to 'flush' things to push out the encoding of the
	 * last few bytes.  There is special encoding if it is not a
	 * multiple of 3
	 */
	BIO_flush(mbio);

  i = BIO_read(bio, out, in_size/4*3);
  out[i]=0;

	/* This call will walk the chain freeing all the BIOs */
	BIO_free_all(bio);

  return i;
}

BF_KEY key;

int SetupPasswordCrypter(char *Passphrase)
{
  BF_set_key(&key, strlen(Passphrase), (unsigned char *)Passphrase); 
  return ERR_OK;
}

void generate_iv(unsigned char *iv, size_t size)
{
  RAND_bytes(iv, size); 
}

int EncryptPassword(char *PlainPassword, char *EncryptedPassword)
{ 
  unsigned char iv[16], local_iv[16];
  char password_b64[4000], iv_b64[32];

  if (strlen(PlainPassword)==0) {
    EncryptedPassword[0]=0;
    return ERR_OK;
  }

  generate_iv(iv, 16);
  memcpy(local_iv, iv, 16);
  BF_cbc_encrypt((unsigned char *)PlainPassword, (unsigned char *)EncryptedPassword,
    strlen(PlainPassword), &key, local_iv, BF_ENCRYPT);

  EncodeBase64(EncryptedPassword, password_b64, strlen(EncryptedPassword)); //really strlen(EncryptedPassword)? it should be something else!)
  EncodeBase64((char *)iv, iv_b64, 16);
  sprintf(EncryptedPassword, "%s|%s", password_b64, iv_b64);
  return ERR_OK;
}

int DecryptPassword(char *EncryptedPassword, char *PlainPassword)
{
  char decoded_password[1000];
  char iv[40];
  char *iv_part = strchr(EncryptedPassword, '|');
  size_t PlainPasswordSize;
  if (!iv_part) {
    strcpy(PlainPassword, EncryptedPassword);
    return ERR_KEY_LOAD_FAILED;
  }
  iv_part[0]=0;
  iv_part++;
  PlainPasswordSize = DecodeBase64(EncryptedPassword, decoded_password, strlen(EncryptedPassword));
  DecodeBase64(iv_part, iv, strlen(iv_part));

  BF_cbc_encrypt((unsigned char *)decoded_password, (unsigned char *)PlainPassword, 
    PlainPasswordSize, &key, (unsigned char *)iv, BF_DECRYPT); 

  return ERR_OK;
}
