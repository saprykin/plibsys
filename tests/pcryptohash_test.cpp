/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

#ifndef PLIB_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pcryptohash_test

#include "plib.h"

#include <string.h>

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

#define PCRYPTO_STRESS_LENGTH	10000
#define PCRYPTO_MAX_UPDATES	1000000

BOOST_AUTO_TEST_CASE (pcryptohash_invalid_test)
{
	PCryptoHash	*hash;
	psize		len;
	pssize		md5_len;
	pchar		*hash_str;
	puchar		*buf;

	p_lib_init ();

	BOOST_CHECK (p_crypto_hash_new ((PCryptoHashType) -1) == NULL);
	BOOST_CHECK (p_crypto_hash_get_length (NULL) == 0);
	BOOST_CHECK (p_crypto_hash_get_string (NULL) == NULL);
	BOOST_CHECK ((pint) p_crypto_hash_get_type (NULL) == -1);

	p_crypto_hash_update (NULL, NULL, 0);
	p_crypto_hash_get_digest (NULL, NULL, NULL);

	p_crypto_hash_get_digest (NULL, NULL, &len);
	BOOST_CHECK (len == 0);

	p_crypto_hash_reset (NULL);

	hash = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_MD5);
	BOOST_CHECK (hash != NULL);

	md5_len = p_crypto_hash_get_length (hash);
	BOOST_CHECK (md5_len > 0);

	buf = (puchar *) p_malloc0 (md5_len);
	BOOST_CHECK (buf != NULL);

	p_crypto_hash_get_digest (hash, buf, &len);
	BOOST_CHECK (len == 0);

	p_crypto_hash_update (hash, (const puchar *) ("abc"), 3);
	len = ((psize) md5_len) - 1;
	p_crypto_hash_get_digest (hash, buf, &len);
	BOOST_CHECK (len == 0);

	hash_str = p_crypto_hash_get_string (hash);
	BOOST_CHECK (strcmp (hash_str, "900150983cd24fb0d6963f7d28e17f72") == 0);
	p_free (hash_str);

	p_crypto_hash_update (hash, (const puchar *) ("abc"), 3);
	hash_str = p_crypto_hash_get_string (hash);
	BOOST_CHECK (strcmp (hash_str, "900150983cd24fb0d6963f7d28e17f72") == 0);
	p_free (hash_str);

	p_crypto_hash_free (hash);
	p_free (buf);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (md5_test)
{
	PCryptoHash	*md5_hash;
	pchar		*hash_str;
	pchar		*long_str;
	puchar		*hash_dig;
	puchar		hash_etalon_1[] = {144,   1,  80, 152,  60, 210,  79, 176,
					   214, 150,  63, 125,  40, 225, 127, 114};
	puchar		hash_etalon_2[] = {130,  21, 239,   7, 150, 162,  11, 202,
					   170, 225,  22, 211, 135, 108, 102,  74};
	puchar		hash_etalon_3[] = {119,   7, 214, 174,  78,   2, 124, 112,
					   238, 162, 169,  53, 194,  41, 111,  33};
	psize		hash_len;

	p_lib_init ();

	md5_hash = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_MD5);

	BOOST_REQUIRE (p_crypto_hash_get_length (md5_hash) == 16);
	BOOST_REQUIRE (p_crypto_hash_get_type (md5_hash) == P_CRYPTO_HASH_TYPE_MD5);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	hash_len = (psize) p_crypto_hash_get_length (md5_hash);
	hash_dig = (puchar *) p_malloc0 (hash_len);
	BOOST_REQUIRE (hash_dig != NULL);

	long_str = (pchar *) p_malloc0 (PCRYPTO_STRESS_LENGTH);
	BOOST_REQUIRE (long_str != NULL);

	for (int i = 0; i < PCRYPTO_STRESS_LENGTH; ++i)
		long_str[i] = (pchar) (97 + i % 20);

	/* Case 1 */

	/* Check string */
	p_crypto_hash_update (md5_hash, (const puchar *) ("abc"), 3);
	hash_str = p_crypto_hash_get_string (md5_hash);
	BOOST_CHECK (strcmp (hash_str, "900150983cd24fb0d6963f7d28e17f72") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	/* Check digest */
	p_crypto_hash_update (md5_hash, (const puchar *) ("abc"), 3);
	p_crypto_hash_get_digest (md5_hash, hash_dig, &hash_len);

	BOOST_CHECK (hash_len == 16);

	for (int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == hash_etalon_1[i]);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	/* Case 2 */

	/* Check string */
	p_crypto_hash_update (md5_hash,
			      (const puchar *) "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			      56);
	hash_str = p_crypto_hash_get_string (md5_hash);
	BOOST_CHECK (strcmp (hash_str, "8215ef0796a20bcaaae116d3876c664a") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	/* Check digest */
	p_crypto_hash_update (md5_hash,
			      (const puchar *) "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			      56);
	p_crypto_hash_get_digest (md5_hash, hash_dig, &hash_len);

	BOOST_CHECK (hash_len == 16);

	for (int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == hash_etalon_2[i]);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	/* Case 3 */

	/* Check string */
	for (int i = 0; i < PCRYPTO_MAX_UPDATES; ++i)
		p_crypto_hash_update (md5_hash, (const puchar *) "a", 1);

	hash_str = p_crypto_hash_get_string (md5_hash);
	BOOST_CHECK (strcmp (hash_str, "7707d6ae4e027c70eea2a935c2296f21") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	/* Check digest */
	for (int i = 0; i < PCRYPTO_MAX_UPDATES; ++i)
		p_crypto_hash_update (md5_hash, (const puchar *) "a", 1);

	p_crypto_hash_get_digest (md5_hash, hash_dig, &hash_len);
	BOOST_CHECK (hash_len == 16);

	for (int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == hash_etalon_3[i]);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	/* Stress test */
	p_crypto_hash_update (md5_hash, (const puchar *) long_str, PCRYPTO_STRESS_LENGTH);
	hash_str = p_crypto_hash_get_string (md5_hash);

	BOOST_CHECK (strcmp (hash_str, "e19ea4a77c97fa6c2521ae1ca66982b9") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (md5_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (md5_hash) == NULL);

	p_free (long_str);
	p_free (hash_dig);
	p_crypto_hash_free (md5_hash);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha1_test)
{
	PCryptoHash	*sha1_hash;
	pchar		*hash_str;
	pchar		*long_str;
	puchar		*hash_dig;
	puchar		hash_etalon_1[] = {169, 153,  62,  54,  71,   6, 129, 106,
					   186,  62,  37, 113, 120,  80, 194, 108,
					   156, 208, 216, 157};
	puchar		hash_etalon_2[] = {132, 152,  62,  68,  28,  59, 210, 110,
					   186, 174,  74, 161, 249,  81,  41, 229,
					   229,  70, 112, 241};
	puchar		hash_etalon_3[] = { 52, 170, 151,  60, 212, 196, 218, 164,
					   246,  30, 235,  43, 219, 173,  39,  49,
					   101,  52,   1, 111};
	psize		hash_len;

	p_lib_init ();

	sha1_hash = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_SHA1);

	BOOST_REQUIRE (p_crypto_hash_get_length (sha1_hash) == 20);
	BOOST_REQUIRE (p_crypto_hash_get_type (sha1_hash) == P_CRYPTO_HASH_TYPE_SHA1);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	hash_len = (psize) p_crypto_hash_get_length (sha1_hash);
	hash_dig = (puchar *) p_malloc0 (hash_len);
	BOOST_REQUIRE (hash_dig != NULL);

	long_str = (pchar *) p_malloc0 (PCRYPTO_STRESS_LENGTH);
	BOOST_REQUIRE (long_str != NULL);

	for (int i = 0; i < PCRYPTO_STRESS_LENGTH; ++i)
		long_str[i] = (pchar) (97 + i % 20);

	/* Case 1 */

	/* Check string */
	p_crypto_hash_update (sha1_hash, (const puchar *) ("abc"), 3);
	hash_str = p_crypto_hash_get_string (sha1_hash);
	BOOST_CHECK (strcmp (hash_str, "a9993e364706816aba3e25717850c26c9cd0d89d") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	/* Check digest */
	p_crypto_hash_update (sha1_hash, (const puchar *) ("abc"), 3);
	p_crypto_hash_get_digest (sha1_hash, hash_dig, &hash_len);

	BOOST_CHECK (hash_len == 20);

	for (int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == hash_etalon_1[i]);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	/* Case 2 */

	/* Check string */
	p_crypto_hash_update (sha1_hash,
			      (const puchar *) ("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
			      56);
	hash_str = p_crypto_hash_get_string (sha1_hash);
	BOOST_CHECK (strcmp (hash_str, "84983e441c3bd26ebaae4aa1f95129e5e54670f1") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	/* Check digest */
	p_crypto_hash_update (sha1_hash,
			      (const puchar *) ("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
			      56);
	p_crypto_hash_get_digest (sha1_hash, hash_dig, &hash_len);

	BOOST_CHECK (hash_len == 20);

	for (int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == hash_etalon_2[i]);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	/* Case 3 */

	/* Check string */
	for (int i = 0; i < PCRYPTO_MAX_UPDATES; ++i)
		p_crypto_hash_update (sha1_hash, (const puchar *) "a", 1);

	hash_str = p_crypto_hash_get_string (sha1_hash);
	BOOST_CHECK (strcmp (hash_str, "34aa973cd4c4daa4f61eeb2bdbad27316534016f") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	/* Check digest */
	for (int i = 0; i < PCRYPTO_MAX_UPDATES; ++i)
		p_crypto_hash_update (sha1_hash, (const puchar *) "a", 1);

	p_crypto_hash_get_digest (sha1_hash, hash_dig, &hash_len);

	BOOST_CHECK (hash_len == 20);

	for (int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == hash_etalon_3[i]);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	/* Stress test */
	p_crypto_hash_update (sha1_hash, (const puchar *) long_str, PCRYPTO_STRESS_LENGTH);
	hash_str = p_crypto_hash_get_string (sha1_hash);

	BOOST_CHECK (strcmp (hash_str, "56309c2dbe04a348ec801ca5f40b035bad01f907") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (sha1_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (sha1_hash) == NULL);

	p_free (long_str);
	p_free (hash_dig);
	p_crypto_hash_free (sha1_hash);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (gost3411_94_test)
{
	PCryptoHash	*gost3411_94_hash;
	pchar		*hash_str;
	pchar		*long_str;
	puchar		*hash_dig;
	puchar		hash_etalon_1[] = {177, 196, 102, 211, 117,  25, 184,  46,
					   131,  25, 129, 159, 243,  37, 149, 224,
					    71, 162, 140, 182, 248,  62, 255,  28,
					   105,  22, 168,  21, 166,  55, 255, 250};
	puchar		hash_etalon_2[] = { 71,  26, 186,  87, 166,  10, 119,  13,
					    58, 118,  19,   6,  53, 193, 251, 234,
					    78, 241,  77, 229,  31, 120, 180, 174,
					    87, 221, 137,  59,  98, 245,  82,   8};
	psize		hash_len;

	p_lib_init ();

	gost3411_94_hash = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_GOST);

	BOOST_REQUIRE (p_crypto_hash_get_length (gost3411_94_hash) == 32);
	BOOST_REQUIRE (p_crypto_hash_get_type (gost3411_94_hash) == P_CRYPTO_HASH_TYPE_GOST);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	hash_len = (psize) p_crypto_hash_get_length (gost3411_94_hash);
	hash_dig = (puchar *) p_malloc0 (hash_len);
	BOOST_REQUIRE (hash_dig != NULL);

	long_str = (pchar *) p_malloc0 (PCRYPTO_STRESS_LENGTH);
	BOOST_REQUIRE (long_str != NULL);

	for (int i = 0; i < PCRYPTO_STRESS_LENGTH; ++i)
		long_str[i] = (pchar) (97 + i % 20);

	/* Case 1 */

	/* Check string */
	p_crypto_hash_update (gost3411_94_hash,
			      (const puchar *) ("This is message, length=32 bytes"),
			      32);
	hash_str = p_crypto_hash_get_string (gost3411_94_hash);
	BOOST_CHECK (strcmp (hash_str, "b1c466d37519b82e8319819ff32595e047a28cb6f83eff1c6916a815a637fffa") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (gost3411_94_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	/* Check digest */
	p_crypto_hash_update (gost3411_94_hash,
			      (const puchar *) ("This is message, length=32 bytes"),
			      32);
	p_crypto_hash_get_digest (gost3411_94_hash, hash_dig, &hash_len);

	BOOST_CHECK (hash_len == 32);

	for (int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == hash_etalon_1[i]);

	p_crypto_hash_reset (gost3411_94_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	/* Case 2 */

	/* Check string */
	p_crypto_hash_update (gost3411_94_hash,
			      (const puchar *) ("Suppose the original message has length = 50 bytes"),
			      50);
	hash_str = p_crypto_hash_get_string (gost3411_94_hash);
	BOOST_CHECK (strcmp (hash_str, "471aba57a60a770d3a76130635c1fbea4ef14de51f78b4ae57dd893b62f55208") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (gost3411_94_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	/* Check digest */
	p_crypto_hash_update (gost3411_94_hash,
			      (const puchar *) ("Suppose the original message has length = 50 bytes"),
			      50);
	p_crypto_hash_get_digest (gost3411_94_hash, hash_dig, &hash_len);

	BOOST_CHECK (hash_len == 32);

	for (int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == hash_etalon_2[i]);

	p_crypto_hash_reset (gost3411_94_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	/* Stress test */
	p_crypto_hash_update (gost3411_94_hash, (const puchar *) long_str, PCRYPTO_STRESS_LENGTH);
	hash_str = p_crypto_hash_get_string (gost3411_94_hash);

	BOOST_CHECK (strcmp (hash_str, "110ddcb6697d508710c64a62f39e7f202d1ffa20314011a0ebaad1281583d77e") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (gost3411_94_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	p_free (long_str);
	p_free (hash_dig);
	p_crypto_hash_free (gost3411_94_hash);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
