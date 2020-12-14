#include "../include/pstruct.h"
#include <string.h>

struct hh_psformat_s hh_make_psformat(const char *format_string)
{
	struct hh_psformat_s output = {
		.format_string = format_string,
		.format_str_chars = strlen(format_string)
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