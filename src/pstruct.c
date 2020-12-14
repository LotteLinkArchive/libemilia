#include "../include/pstruct.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

/* Endian conversion for uint64_t/int64_t */
#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32)
# define ntohll(x) ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32)
#endif

/* Representation of a type */
struct hh_pstype_s {
	char type;

	size_t bytes;

	bool is_variable;
	bool is_valid;
};

/* This function just does type property recognition. It finds out the validity, variableness and size of a type. */
struct hh_pstype_s hh_pstype_get(char type)
{
	struct hh_pstype_s output = {.type = type, .is_valid = true, .is_variable = true};

	switch (type) {
	case HH_PSTYPE_PAD:    output.bytes = 1; output.is_variable = false; break;
	case HH_PSTYPE_U8:     output.bytes = 1; break;
	case HH_PSTYPE_I8:     output.bytes = 1; break;
	case HH_PSTYPE_BOOL:   output.bytes = 1; break;
	case HH_PSTYPE_U16:    output.bytes = 2; break;
	case HH_PSTYPE_I16:    output.bytes = 2; break;
	case HH_PSTYPE_U32:    output.bytes = 4; break;
	case HH_PSTYPE_I32:    output.bytes = 4; break;
	case HH_PSTYPE_U64:    output.bytes = 8; break;
	case HH_PSTYPE_I64:    output.bytes = 8; break;
	case HH_PSTYPE_FLOAT:  output.bytes = 4; break;
	case HH_PSTYPE_DOUBLE: output.bytes = 8; break;
	default:               output.is_valid = false; output.is_variable = false; break;
	}

	return output;
}

struct hh_psformat_s hh_make_psformat(const char *format_string)
{
	struct hh_psformat_s output = {
		.format_string = format_string,
		.format_str_chars = strlen(format_string),
		.status = HH_STATUS_OKAY
	};

	for (;;) {
		struct hh_pstype_s cproc = hh_pstype_get(*format_string++);
		if (cproc.type == '\0') break;
		if (!cproc.is_valid) {
			output.status = HH_INVALID_TYPE;
			break;
		}
		if (cproc.is_variable) output.variables++;
		output.data_length += cproc.bytes;
	}

	return output;
}

struct hh_psbuf_s hh_psmkbuf(struct hh_psformat_s *format, void *data)
{
	struct hh_psbuf_s output = {
		.format = format,
		.status = HH_STATUS_OKAY
	};

	output.fields = malloc(format->variables * sizeof(struct hh_psfield_s));
	if (!output.fields) goto hh_psmkbuf_oom;
	memset(output.fields, 0, format->variables * sizeof(struct hh_psfield_s));

	output.buffer = malloc(format->data_length);
	if (!output.buffer) goto hh_psmkbuf_oom;
	if (data) memcpy(output.buffer, data, format->data_length);
	else memset(output.buffer, 0, format->data_length);
	uint8_t *bdata = output.buffer;

	const char *format_string = format->format_string;
	unsigned int field_index = 0;

	for (;;) {
		struct hh_pstype_s cproc = hh_pstype_get(*format_string++);

		if (cproc.type == '\0') break;
		if (!cproc.is_valid) continue;
		if (!cproc.is_variable) goto hh_psmkbuf_mvnb;

		output.fields[field_index].type = cproc.type;
		output.fields[field_index].bytes = cproc.bytes;
		
		output.fields[field_index].data = bdata;

		field_index++;
hh_psmkbuf_mvnb:
		bdata = bdata + cproc.bytes;
	}

hh_psmkbuf_exit:
	return output;
hh_psmkbuf_oom:
	output.status = HH_OUT_OF_MEMORY;
	goto hh_psmkbuf_exit;
}

void hh_psupdbuf(struct hh_psbuf_s buffer, void *data)
{
	memcpy(buffer.buffer, data, buffer.format->data_length);
}

hh_status_t hh_psfreebuf(struct hh_psbuf_s buffer)
{
	if (!buffer.fields || !buffer.buffer) return HH_DOUBLE_FREE;

	free(buffer.buffer);
	free(buffer.fields);
	buffer.buffer = NULL;
	buffer.fields = NULL;

	return HH_STATUS_OKAY;
}

void hh_psfield_set(struct hh_psbuf_s buffer, unsigned int index, union hh_pstypebuf_u value)
{
	switch (buffer.fields[index].type) {
		case HH_PSTYPE_U16:
		case HH_PSTYPE_I16:
			value.uint16 = htons(value.uint16);
			break;
		case HH_PSTYPE_FLOAT:
		case HH_PSTYPE_U32:
		case HH_PSTYPE_I32:
			value.uint32 = htonl(value.uint32);
			break;
		case HH_PSTYPE_DOUBLE:
		case HH_PSTYPE_U64:
		case HH_PSTYPE_I64:
			value.uint64 = htonll(value.uint64);
			break;
		default:
			break;
	}

	memcpy(buffer.fields[index].data, &value, buffer.fields[index].bytes);
}

union hh_pstypebuf_u hh_psfield_get(struct hh_psbuf_s buffer, unsigned int index)
{
	union hh_pstypebuf_u value;
	memcpy(&value, buffer.fields[index].data, buffer.fields[index].bytes);

	switch (buffer.fields[index].type) {
		case HH_PSTYPE_U16:
		case HH_PSTYPE_I16:
			value.uint16 = ntohs(value.uint16);
			break;
		case HH_PSTYPE_FLOAT:
		case HH_PSTYPE_U32:
		case HH_PSTYPE_I32:
			value.uint32 = ntohl(value.uint32);
			break;
		case HH_PSTYPE_DOUBLE:
		case HH_PSTYPE_U64:
		case HH_PSTYPE_I64:
			value.uint64 = ntohll(value.uint64);
			break;
		default:
			break;
	}

	return value;
}