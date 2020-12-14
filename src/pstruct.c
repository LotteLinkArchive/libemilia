#include "../include/pstruct.h"
#include <string.h>
#include <stdlib.h>

struct hh_psformat_s hh_make_psformat(const char *format_string)
{
	struct hh_psformat_s output = {
		.format_string = format_string,
		.format_str_chars = strlen(format_string),
		.status = HH_STATUS_OKAY
	};

	for (;;) {
		#define VARINC(size) output.data_length += size; output.variables++
		switch (*format_string++) {
		case '\0':             goto hh_psformat_jlexp;
		case HH_PSTYPE_PAD:    output.data_length++; break;
		case HH_PSTYPE_U8:     VARINC(1); break;
		case HH_PSTYPE_I8:     VARINC(1); break;
		case HH_PSTYPE_BOOL:   VARINC(1); break;
		case HH_PSTYPE_U16:    VARINC(2); break;
		case HH_PSTYPE_I16:    VARINC(2); break;
		case HH_PSTYPE_U32:    VARINC(4); break;
		case HH_PSTYPE_I32:    VARINC(4); break;
		case HH_PSTYPE_U64:    VARINC(8); break;
		case HH_PSTYPE_I64:    VARINC(8); break;
		case HH_PSTYPE_FLOAT:  VARINC(4); break;
		case HH_PSTYPE_DOUBLE: VARINC(8); break;
		default:               output.status = HH_INVALID_TYPE; goto hh_psformat_jlexp;
		}
		#undef VARINC
	}
hh_psformat_jlexp:
	return output;
}

struct hh_psbuf_s hh_psmkbuf(struct hh_psformat_s *format, void *data)
{
	struct hh_psbuf_s output = {
		.format = format,
		.allocated_by_us = false,
		.status = HH_STATUS_OKAY
	};

	if (data) output.data = data; goto hh_psmkbuf_done;

	output.allocated_by_us = true;
	output.data = malloc(format->data_length);
	if (!data) output.status = HH_OUT_OF_MEMORY; goto hh_psmkbuf_done;
	
	memset(output.data, 0, format->data_length);
hh_psmkbuf_done:
	return output;
}

hh_status_t hh_psfreebuf(struct hh_psbuf_s buffer, bool remote_override)
{
	if ((buffer.allocated_by_us || remote_override) && buffer.data) {
		free(buffer.data);
		buffer.data = NULL;
		return HH_STATUS_OKAY;
	}

	if (!buffer.data) return HH_DOUBLE_FREE;
	
	return HH_REMOTE_ALLOC;
}
