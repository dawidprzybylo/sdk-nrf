/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <zephyr/ztest.h>
#include <nfc/ndef/record_parser.h>

static int invoke_record_parse(const uint8_t *nfc_data, uint32_t nfc_data_len,
			       struct nfc_ndef_bin_payload_desc *bin_pay_desc,
			       struct nfc_ndef_record_desc *rec_desc,
			       enum nfc_ndef_record_location *record_location)
{
	uint32_t len = nfc_data_len;

	return nfc_ndef_record_parse(bin_pay_desc, rec_desc, record_location, nfc_data, &len);
}

ZTEST(nfc_ndef_record_parser_unittest, test_valid_short_record)
{
	static const uint8_t record[] = {
		NDEF_LONE_RECORD | TNF_WELL_KNOWN | NDEF_RECORD_SR_MASK,
		0x01,
		0x03,
		'U',
		'a',
		'b',
		'c',
	};

	struct nfc_ndef_bin_payload_desc bin_pay_desc;
	struct nfc_ndef_record_desc rec_desc;
	enum nfc_ndef_record_location record_location;
	uint32_t len = sizeof(record);
	int err;

	err = invoke_record_parse(record, len, &bin_pay_desc, &rec_desc, &record_location);
	zassert_ok(err, NULL);
	zassert_equal(record_location, NDEF_LONE_RECORD, NULL);
	zassert_equal(rec_desc.type_length, 1, NULL);
	zassert_equal(bin_pay_desc.payload_length, 3, NULL);
	zassert_equal_ptr(rec_desc.type, &record[3], NULL);
	zassert_equal_ptr(bin_pay_desc.payload, &record[4], NULL);
}

ZTEST(nfc_ndef_record_parser_unittest, test_rejects_overflowing_long_payload_length)
{
	/* Long-format record in a 16-byte buffer. A wrapped size check used to pass. */
	static const uint8_t buffer[16] = {
		NDEF_LONE_RECORD | TNF_WELL_KNOWN,
		0x01,
		0xFF,
		0xFF,
		0xFF,
		0xF9,
		'U',
	};

	struct nfc_ndef_bin_payload_desc bin_pay_desc;
	struct nfc_ndef_record_desc rec_desc;
	enum nfc_ndef_record_location record_location;
	uint32_t len = sizeof(buffer);
	int err;

	err = invoke_record_parse(buffer, len, &bin_pay_desc, &rec_desc, &record_location);
	zassert_equal(err, -EINVAL, NULL);
}

ZTEST(nfc_ndef_record_parser_unittest, test_rejects_payload_larger_than_buffer)
{
	static const uint8_t record[] = {
		NDEF_LONE_RECORD | TNF_WELL_KNOWN | NDEF_RECORD_SR_MASK,
		0x01,
		0x08,
		'U',
	};

	struct nfc_ndef_bin_payload_desc bin_pay_desc;
	struct nfc_ndef_record_desc rec_desc;
	enum nfc_ndef_record_location record_location;
	int err;

	err = invoke_record_parse(record, sizeof(record), &bin_pay_desc, &rec_desc,
				  &record_location);
	zassert_equal(err, -EINVAL, NULL);
}

ZTEST_SUITE(nfc_ndef_record_parser_unittest, NULL, NULL, NULL, NULL, NULL);
