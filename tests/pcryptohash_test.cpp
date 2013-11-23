#define BOOST_TEST_MODULE pcryptohash_test

#include "plib.h"

#include <string.h>

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pcryptohash_general_test)
{
	PCryptoHash *hash = NULL;

	p_crypto_hash_get_digest (hash, NULL, NULL);
	BOOST_REQUIRE (p_crypto_hash_get_length (hash) == 0);
	BOOST_REQUIRE (p_crypto_hash_get_string (hash) == NULL);
	BOOST_REQUIRE (p_crypto_hash_get_type (hash) == -1);
	p_crypto_hash_reset (hash);
}

BOOST_AUTO_TEST_CASE (md5_test)
{
	PCryptoHash	*md5_hash;
	pchar		*hash_str;
	pint		i;

	md5_hash = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_MD5);

	BOOST_REQUIRE (p_crypto_hash_get_length (md5_hash) == 16);
	BOOST_REQUIRE (p_crypto_hash_get_type (md5_hash) == P_CRYPTO_HASH_TYPE_MD5);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	p_crypto_hash_update (md5_hash, (const puchar *) ("abc"), 3);
	hash_str = p_crypto_hash_get_string (md5_hash);
	BOOST_CHECK (strcmp (hash_str, "900150983cd24fb0d6963f7d28e17f72") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	p_crypto_hash_update (md5_hash,
			      (const puchar *) "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			      56);
	hash_str = p_crypto_hash_get_string (md5_hash);
	BOOST_CHECK (strcmp (hash_str, "8215ef0796a20bcaaae116d3876c664a") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	for (i = 0; i < 1000000; ++i)
		p_crypto_hash_update (md5_hash, (const puchar *) "a", 1);
	hash_str = p_crypto_hash_get_string (md5_hash);
	BOOST_CHECK (strcmp (hash_str, "7707d6ae4e027c70eea2a935c2296f21") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	p_crypto_hash_free (md5_hash);
}

BOOST_AUTO_TEST_CASE (sha1_test)
{
	PCryptoHash	*sha1_hash;
	pchar		*hash_str;
	pint		i;

	sha1_hash = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_SHA1);

	BOOST_REQUIRE (p_crypto_hash_get_length (sha1_hash) == 20);
	BOOST_REQUIRE (p_crypto_hash_get_type (sha1_hash) == P_CRYPTO_HASH_TYPE_SHA1);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	p_crypto_hash_update (sha1_hash, (const puchar *) ("abc"), 3);
	hash_str = p_crypto_hash_get_string (sha1_hash);
	BOOST_CHECK (strcmp (hash_str, "a9993e364706816aba3e25717850c26c9cd0d89d") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	p_crypto_hash_update (sha1_hash,
			      (const puchar *) ("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
			      56);
	hash_str = p_crypto_hash_get_string (sha1_hash);
	BOOST_CHECK (strcmp (hash_str, "84983e441c3bd26ebaae4aa1f95129e5e54670f1") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	for (i = 0; i < 1000000; ++i)
		p_crypto_hash_update (sha1_hash, (const puchar *) "a", 1);
	hash_str = p_crypto_hash_get_string (sha1_hash);
	BOOST_CHECK (strcmp (hash_str, "34aa973cd4c4daa4f61eeb2bdbad27316534016f") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	p_crypto_hash_free (sha1_hash);
}

BOOST_AUTO_TEST_CASE (gost3411_94_test)
{
	PCryptoHash	*gost3411_94_hash;
	pchar		*hash_str;

	gost3411_94_hash = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_GOST);

	BOOST_REQUIRE (p_crypto_hash_get_length (gost3411_94_hash) == 32);
	BOOST_REQUIRE (p_crypto_hash_get_type (gost3411_94_hash) == P_CRYPTO_HASH_TYPE_GOST);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	p_crypto_hash_update (gost3411_94_hash,
			      (const puchar *) ("This is message, length=32 bytes"),
			      32);
	hash_str = p_crypto_hash_get_string (gost3411_94_hash);
	BOOST_CHECK (strcmp (hash_str, "b1c466d37519b82e8319819ff32595e047a28cb6f83eff1c6916a815a637fffa") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (gost3411_94_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	p_crypto_hash_update (gost3411_94_hash,
			      (const puchar *) ("Suppose the original message has length = 50 bytes"),
			      50);
	hash_str = p_crypto_hash_get_string (gost3411_94_hash);
	BOOST_CHECK (strcmp (hash_str, "471aba57a60a770d3a76130635c1fbea4ef14de51f78b4ae57dd893b62f55208") == 0);
	p_free (hash_str);

	p_crypto_hash_free (gost3411_94_hash);
}

BOOST_AUTO_TEST_SUITE_END()
