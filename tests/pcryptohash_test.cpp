/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pcryptohash_test

#include "plibsys.h"

#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

#define PCRYPTO_STRESS_LENGTH	10000
#define PCRYPTO_MAX_UPDATES	1000000

extern "C" ppointer pmem_alloc (psize nbytes)
{
	P_UNUSED (nbytes);
	return (ppointer) NULL;
}

extern "C" ppointer pmem_realloc (ppointer block, psize nbytes)
{
	P_UNUSED (block);
	P_UNUSED (nbytes);
	return (ppointer) NULL;
}

extern "C" void pmem_free (ppointer block)
{
	P_UNUSED (block);
}

static void
general_hash_test (PCryptoHashType	type,
		   psize		hash_len,
		   const pchar		*msg1,
		   const pchar		*msg2,
		   const puchar		*etalon1,
		   const puchar		*etalon2,
		   const puchar		*etalon3,
		   const pchar		*hash1,
		   const pchar		*hash2,
		   const pchar		*hash3,
		   const pchar		*hash_stress)
{
	PCryptoHash	*crypto_hash;
	psize		dig_len;
	pchar		*hash_str;
	pchar		*long_str;
	puchar		*hash_dig;

	crypto_hash = p_crypto_hash_new (type);

	BOOST_REQUIRE ((psize) p_crypto_hash_get_length (crypto_hash) == hash_len);
	BOOST_REQUIRE (p_crypto_hash_get_type (crypto_hash) == type);
	BOOST_REQUIRE (p_crypto_hash_get_string (crypto_hash) == NULL);

	hash_dig = (puchar *) p_malloc0 (hash_len);
	BOOST_REQUIRE (hash_dig != NULL);

	long_str = (pchar *) p_malloc0 (PCRYPTO_STRESS_LENGTH);
	BOOST_REQUIRE (long_str != NULL);

	for (int i = 0; i < PCRYPTO_STRESS_LENGTH; ++i)
		long_str[i] = (pchar) (97 + i % 20);

	/* Case 1 */

	/* Check string */
	p_crypto_hash_update (crypto_hash, (const puchar *) msg1, strlen (msg1));
	hash_str = p_crypto_hash_get_string (crypto_hash);
	BOOST_CHECK (strcmp (hash_str, hash1) == 0);
	p_free (hash_str);

	p_crypto_hash_reset (crypto_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (crypto_hash) == NULL);

	/* Check digest */
	dig_len = hash_len;
	p_crypto_hash_update (crypto_hash, (const puchar *) msg1, strlen (msg1));
	p_crypto_hash_get_digest (crypto_hash, hash_dig, &dig_len);

	BOOST_CHECK (dig_len == hash_len);

	for (unsigned int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == etalon1[i]);

	p_crypto_hash_reset (crypto_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (crypto_hash) == NULL);

	/* Case 2 */

	/* Check string */
	p_crypto_hash_update (crypto_hash, (const puchar *) msg2, strlen (msg2));
	hash_str = p_crypto_hash_get_string (crypto_hash);
	BOOST_CHECK (strcmp (hash_str, hash2) == 0);
	p_free (hash_str);

	p_crypto_hash_reset (crypto_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (crypto_hash) == NULL);

	/* Check digest */
	dig_len = hash_len;
	p_crypto_hash_update (crypto_hash, (const puchar *) msg2, strlen (msg2));
	p_crypto_hash_get_digest (crypto_hash, hash_dig, &dig_len);

	BOOST_CHECK (dig_len == hash_len);

	for (unsigned int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == etalon2[i]);

	p_crypto_hash_reset (crypto_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (crypto_hash) == NULL);

	/* Case 3 */

	/* Check string */
	for (int i = 0; i < PCRYPTO_MAX_UPDATES; ++i)
		p_crypto_hash_update (crypto_hash, (const puchar *) "a", 1);

	hash_str = p_crypto_hash_get_string (crypto_hash);

	BOOST_CHECK (strcmp (hash_str, hash3) == 0);
	p_free (hash_str);

	p_crypto_hash_reset (crypto_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (crypto_hash) == NULL);

	/* Check digest */
	dig_len = hash_len;
	for (int i = 0; i < PCRYPTO_MAX_UPDATES; ++i)
		p_crypto_hash_update (crypto_hash, (const puchar *) "a", 1);

	p_crypto_hash_get_digest (crypto_hash, hash_dig, &dig_len);
	BOOST_CHECK (dig_len == hash_len);

	for (unsigned int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == etalon3[i]);

	p_crypto_hash_reset (crypto_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (crypto_hash) == NULL);

	/* Stress test */
	p_crypto_hash_update (crypto_hash, (const puchar *) long_str, PCRYPTO_STRESS_LENGTH);
	hash_str = p_crypto_hash_get_string (crypto_hash);

	BOOST_CHECK (strcmp (hash_str, hash_stress) == 0);
	p_free (hash_str);

	p_crypto_hash_reset (crypto_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (crypto_hash) == NULL);

	p_free (long_str);
	p_free (hash_dig);
	p_crypto_hash_free (crypto_hash);
}

BOOST_AUTO_TEST_CASE (pcryptohash_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	BOOST_CHECK (p_crypto_hash_new (P_CRYPTO_HASH_TYPE_MD5) == NULL);
	BOOST_CHECK (p_crypto_hash_new (P_CRYPTO_HASH_TYPE_SHA1) == NULL);
	BOOST_CHECK (p_crypto_hash_new (P_CRYPTO_HASH_TYPE_GOST) == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pcryptohash_invalid_test)
{
	PCryptoHash	*hash;
	psize		len;
	pssize		md5_len;
	pchar		*hash_str;
	puchar		*buf;

	p_libsys_init ();

	BOOST_CHECK (p_crypto_hash_new ((PCryptoHashType) -1) == NULL);
	BOOST_CHECK (p_crypto_hash_get_length (NULL) == 0);
	BOOST_CHECK (p_crypto_hash_get_string (NULL) == NULL);
	BOOST_CHECK ((pint) p_crypto_hash_get_type (NULL) == -1);
	p_crypto_hash_free (NULL);

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

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (md5_test)
{
	const puchar	hash_etalon_1[] = {144,   1,  80, 152,  60, 210,  79, 176,
					   214, 150,  63, 125,  40, 225, 127, 114};
	const puchar	hash_etalon_2[] = {130,  21, 239,   7, 150, 162,  11, 202,
					   170, 225,  22, 211, 135, 108, 102,  74};
	const puchar	hash_etalon_3[] = {119,   7, 214, 174,  78,   2, 124, 112,
					   238, 162, 169,  53, 194,  41, 111,  33};

	p_libsys_init ();

	general_hash_test (P_CRYPTO_HASH_TYPE_MD5,
			   16,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "900150983cd24fb0d6963f7d28e17f72",
			   "8215ef0796a20bcaaae116d3876c664a",
			   "7707d6ae4e027c70eea2a935c2296f21",
			   "e19ea4a77c97fa6c2521ae1ca66982b9");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha1_test)
{
	const puchar	hash_etalon_1[] = {169, 153,  62,  54,  71,   6, 129, 106,
					   186,  62,  37, 113, 120,  80, 194, 108,
					   156, 208, 216, 157};
	const puchar	hash_etalon_2[] = {132, 152,  62,  68,  28,  59, 210, 110,
					   186, 174,  74, 161, 249,  81,  41, 229,
					   229,  70, 112, 241};
	const puchar	hash_etalon_3[] = { 52, 170, 151,  60, 212, 196, 218, 164,
					   246,  30, 235,  43, 219, 173,  39,  49,
					   101,  52,   1, 111};

	p_libsys_init ();

	general_hash_test (P_CRYPTO_HASH_TYPE_SHA1,
			   20,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "a9993e364706816aba3e25717850c26c9cd0d89d",
			   "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
			   "34aa973cd4c4daa4f61eeb2bdbad27316534016f",
			   "56309c2dbe04a348ec801ca5f40b035bad01f907");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha2_224_test)
{
	const puchar	hash_etalon_1[] = { 35,   9, 125,  34,  52,   5, 216,  34, 134,  66,
					   164, 119, 189, 162,  85, 179,  42, 173, 188, 228,
					   189, 160, 179, 247, 227, 108, 157, 167};
	const puchar	hash_etalon_2[] = {117,  56, 139,  22,  81,  39, 118, 204,  93, 186,
					    93, 161, 253, 137,   1,  80, 176, 198,  69,  92,
					   180, 245, 139,  25,  82,  82,  37,  37};
	const puchar	hash_etalon_3[] = { 32, 121,  70,  85, 152,  12, 145, 216, 187, 180,
					   193, 234, 151,  97, 138,  75, 240,  63,  66,  88,
					    25,  72, 178, 238,  78, 231, 173, 103, };

	p_libsys_init ();

	general_hash_test (P_CRYPTO_HASH_TYPE_SHA2_224,
			   28,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7",
			   "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525",
			   "20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67",
			   "4cf3d45b57e0d54981c4d86954e8378168d5a9f6ceab9e0aae5dd2f6");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha2_256_test)
{
	const puchar	hash_etalon_1[] = {186, 120,  22, 191, 143,   1, 207, 234,
					    65,  65,  64, 222,  93, 174,  34,  35,
					   176,   3,  97, 163, 150,  23, 122, 156,
					   180,  16, 255,  97, 242,   0,  21, 173};
	const puchar	hash_etalon_2[] = { 36, 141, 106,  97, 210,   6,  56, 184,
					   229, 192,  38, 147,  12,  62,  96,  57,
					   163,  60, 228,  89, 100, 255,  33, 103,
					   246, 236, 237, 212,  25, 219,   6, 193};
	const puchar	hash_etalon_3[] = {205, 199, 110,  92, 153,  20, 251, 146,
					   129, 161, 199, 226, 132, 215,  62, 103,
					   241, 128, 154,  72, 164, 151,  32,  14,
					     4, 109,  57, 204, 199,  17,  44, 208};

	p_libsys_init ();

	general_hash_test (P_CRYPTO_HASH_TYPE_SHA2_256,
			   32,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
			   "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
			   "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
			   "4c2d7749e1b711ca652fda20dd29fe378fd9988f19eadadfa570682e2c55349f");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (gost3411_94_test)
{
	PCryptoHash	*gost3411_94_hash;
	pchar		*hash_str;
	const puchar	hash_etalon_1[] = {177, 196, 102, 211, 117,  25, 184,  46,
					   131,  25, 129, 159, 243,  37, 149, 224,
					    71, 162, 140, 182, 248,  62, 255,  28,
					   105,  22, 168,  21, 166,  55, 255, 250};
	const puchar	hash_etalon_2[] = { 71,  26, 186,  87, 166,  10, 119,  13,
					    58, 118,  19,   6,  53, 193, 251, 234,
					    78, 241,  77, 229,  31, 120, 180, 174,
					    87, 221, 137,  59,  98, 245,  82,   8};
	const puchar	hash_etalon_3[] = { 92,   0, 204, 194, 115,  76, 221,  51,
					    50, 211, 212, 116, 149, 118, 227, 193,
					   167, 219, 175,  14, 126, 167,  78, 159,
					   166,   2,  65,  60, 144, 161,  41, 250};

	p_libsys_init ();

	general_hash_test (P_CRYPTO_HASH_TYPE_GOST,
			   32,
			   "This is message, length=32 bytes",
			   "Suppose the original message has length = 50 bytes",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "b1c466d37519b82e8319819ff32595e047a28cb6f83eff1c6916a815a637fffa",
			   "471aba57a60a770d3a76130635c1fbea4ef14de51f78b4ae57dd893b62f55208",
			   "5c00ccc2734cdd3332d3d4749576e3c1a7dbaf0e7ea74e9fa602413c90a129fa",
			   "110ddcb6697d508710c64a62f39e7f202d1ffa20314011a0ebaad1281583d77e");

	gost3411_94_hash = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_GOST);

	BOOST_REQUIRE (gost3411_94_hash != NULL);

	/* Repeat test */
	p_crypto_hash_update (gost3411_94_hash, (const puchar *) "message digest", 14);
	p_crypto_hash_update (gost3411_94_hash, (const puchar *) "message digest", 14);
	p_crypto_hash_update (gost3411_94_hash, (const puchar *) "message digest", 14);
	p_crypto_hash_update (gost3411_94_hash, (const puchar *) "message digest", 14);

	hash_str = p_crypto_hash_get_string (gost3411_94_hash);
	BOOST_CHECK (strcmp (hash_str, "1564064cce4fe1386be063f98d7ab17fc724fa7f02be4fa6847a2162be20d807") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (gost3411_94_hash);
	BOOST_REQUIRE (p_crypto_hash_get_string (gost3411_94_hash) == NULL);

	p_crypto_hash_free (gost3411_94_hash);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
