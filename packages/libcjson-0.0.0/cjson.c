#include <cjson.h>

#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT		(1000)
#endif

#ifndef CJSON_CIRCULAR_LIMIT
#define CJSON_CIRCULAR_LIMIT	(10000)
#endif

#ifndef CJSON_STATIC_STRLEN
#define CJSON_STATIC_STRLEN(s)	(sizeof(s) - sizeof(""))
#endif

#ifndef CJSON_ISNAN
#define CJSON_ISNAN(d)			(d != d)
#endif

#ifndef CJSON_ISINF
#define CJSON_ISINF(d)			(CJSON_ISNAN((d - d)) && !CJSON_ISNAN(d))
#endif

#ifndef CJSON_NAN
#define CJSON_NAN				(0.0 / 0.0)
#endif

static int case_insensitive_strcmp(const unsigned char * s1, const unsigned char * s2)
{
	if((s1 == NULL) || (s2 == NULL))
		return 1;
	if(s1 == s2)
		return 0;
	for(; xos_tolower(*s1) == xos_tolower(*s2); (void)s1++, s2++)
	{
		if(*s1 == '\0')
			return 0;
	}
	return xos_tolower(*s1) - xos_tolower(*s2);
}

static unsigned char * cjson_strdup(const unsigned char * str)
{
	unsigned char * cpy = NULL;
	size_t len = 0;

	if(str == NULL)
		return NULL;
	len = xos_strlen((const char *)str) + sizeof("");
	cpy = (unsigned char *)xos_mem_malloc(len);
	if(cpy == NULL)
		return NULL;
	xos_memcpy(cpy, str, len);
	return cpy;
}

static struct cjson_t * cjson_new_item(void)
{
	struct cjson_t * node = (struct cjson_t *)xos_mem_malloc(sizeof(struct cjson_t));
	if(node)
		xos_memset(node, 0, sizeof(struct cjson_t));
	return node;
}

char * cjson_get_string_value(const struct cjson_t * item)
{
	if(!cjson_is_string(item))
		return NULL;
	return item->valuestring;
}

double cjson_get_number_value(const struct cjson_t * item)
{
	if(!cjson_is_number(item))
		return (double)CJSON_NAN;
	return item->valuedouble;
}

void cjson_delete(struct cjson_t * item)
{
	struct cjson_t * next = NULL;
	while(item != NULL)
	{
		next = item->next;
		if(!(item->type & CJSON_ISREFERENCE) && (item->child != NULL))
		{
			cjson_delete(item->child);
		}
		if(!(item->type & CJSON_ISREFERENCE) && (item->valuestring != NULL))
		{
			xos_mem_free(item->valuestring);
			item->valuestring = NULL;
		}
		if(!(item->type & CJSON_STRINGISCONST) && (item->string != NULL))
		{
			xos_mem_free(item->string);
			item->string = NULL;
		}
		xos_mem_free(item);
		item = next;
	}
}

struct parse_buffer_t {
	const unsigned char * content;
	size_t length;
	size_t offset;
	size_t depth;
};

#define can_read(buffer, size)					((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
#define can_access_at_index(buffer, index)		((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index)	(!can_access_at_index(buffer, index))
#define buffer_at_offset(buffer)				((buffer)->content + (buffer)->offset)

static int parse_number(struct cjson_t * item, struct parse_buffer_t * input_buffer)
{
	double number = 0;
	unsigned char * after_end = NULL;
	unsigned char * number_c_string;
	unsigned char decimal_point = '.';
	size_t i = 0;
	size_t number_string_length = 0;
	int has_decimal_point = 0;

	if((input_buffer == NULL) || (input_buffer->content == NULL))
		return 0;
	for(i = 0; can_access_at_index(input_buffer, i); i++)
	{
		switch(buffer_at_offset(input_buffer)[i])
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '+':
		case '-':
		case 'e':
		case 'E':
			number_string_length++;
			break;

		case '.':
			number_string_length++;
			has_decimal_point = 1;
			break;

		default:
			goto loop_end;
		}
	}
loop_end:
	number_c_string = (unsigned char *)xos_mem_malloc(number_string_length + 1);
	if(number_c_string == NULL)
		return 0;
	xos_memcpy(number_c_string, buffer_at_offset(input_buffer), number_string_length);
	number_c_string[number_string_length] = '\0';
	if(has_decimal_point)
	{
		for(i = 0; i < number_string_length; i++)
		{
			if(number_c_string[i] == '.')
				number_c_string[i] = decimal_point;
		}
	}
	number = xos_strtod((const char*)number_c_string, (char**)&after_end);
	if(number_c_string == after_end)
	{
		xos_mem_free(number_c_string);
		return 0;
	}
	item->valuedouble = number;
	if(number >= INT_MAX)
		item->valueint = INT_MAX;
	else if(number <= (double)INT_MIN)
		item->valueint = INT_MIN;
	else
		item->valueint = (int)number;
	item->type = CJSON_NUMBER;
	input_buffer->offset += (size_t)(after_end - number_c_string);
	xos_mem_free(number_c_string);
	return 1;
}

double cjson_set_number_helper(struct cjson_t * object, double number)
{
	if(object == NULL)
		return (double)CJSON_NAN;
	if(number >= INT_MAX)
		object->valueint = INT_MAX;
	else if(number <= (double)INT_MIN)
		object->valueint = INT_MIN;
	else
		object->valueint = (int)number;
	return object->valuedouble = number;
}

char * cjson_set_valuestring(struct cjson_t * object, const char * valuestring)
{
	char * copy = NULL;
	size_t v1_len;
	size_t v2_len;
	if((object == NULL) || !(object->type & CJSON_STRING) || (object->type & CJSON_ISREFERENCE))
		return NULL;
	if(object->valuestring == NULL || valuestring == NULL)
		return NULL;

	v1_len = xos_strlen(valuestring);
	v2_len = xos_strlen(object->valuestring);

	if(v1_len <= v2_len)
	{
		if(!(valuestring + v1_len < object->valuestring || object->valuestring + v2_len < valuestring))
			return NULL;
		xos_strcpy(object->valuestring, valuestring);
		return object->valuestring;
	}
	copy = (char *)cjson_strdup((const unsigned char *)valuestring);
	if(copy == NULL)
		return NULL;
	if(object->valuestring != NULL)
	{
		xos_mem_free(object->valuestring);
		object->valuestring = NULL;
	}
	object->valuestring = copy;

	return copy;
}

struct printbuffer_t {
	unsigned char * buffer;
	size_t length;
	size_t offset;
	size_t depth;
	int noalloc;
	int format;
};

static unsigned char * ensure(struct printbuffer_t * p, size_t needed)
{
	unsigned char * newbuffer = NULL;
	size_t newsize = 0;

	if((p == NULL) || (p->buffer == NULL))
		return NULL;
	if((p->length > 0) && (p->offset >= p->length))
		return NULL;
	if(needed > INT_MAX)
		return NULL;
	needed += p->offset + 1;
	if(needed <= p->length)
		return p->buffer + p->offset;
	if(p->noalloc)
		return NULL;
	if(needed > (INT_MAX / 2))
	{
		if(needed <= INT_MAX)
			newsize = INT_MAX;
		else
			return NULL;
	}
	else
		newsize = needed * 2;
	newbuffer = (unsigned char *)xos_mem_realloc(p->buffer, newsize);
	if(newbuffer == NULL)
	{
		xos_mem_free(p->buffer);
		p->length = 0;
		p->buffer = NULL;
		return NULL;
	}
	p->length = newsize;
	p->buffer = newbuffer;

	return newbuffer + p->offset;
}

static void update_offset(struct printbuffer_t * buffer)
{
	const unsigned char * buffer_pointer = NULL;
	if((buffer == NULL) || (buffer->buffer == NULL))
		return;
	buffer_pointer = buffer->buffer + buffer->offset;
	buffer->offset += xos_strlen((const char*)buffer_pointer);
}

static int compare_double(double a, double b)
{
	double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
	return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

static int print_number(const struct cjson_t * item, struct printbuffer_t * output_buffer)
{
	unsigned char * output_pointer = NULL;
	double d = item->valuedouble;
	int length = 0;
	size_t i = 0;
	unsigned char number_buffer[26] = { 0 };
	unsigned char decimal_point = '.';
	double test = 0.0;

	if(output_buffer == NULL)
		return 0;
	if(CJSON_ISNAN(d) || CJSON_ISINF(d))
		length = xos_sprintf((char*)number_buffer, "null");
	else if(d == (double)item->valueint)
		length = xos_sprintf((char*)number_buffer, "%d", item->valueint);
	else
	{
		length = xos_sprintf((char*)number_buffer, "%1.15g", d);
		if((xos_sscanf((char*)number_buffer, "%lg", &test) != 1) || !compare_double((double)test, d))
			length = xos_sprintf((char*)number_buffer, "%1.17g", d);
	}
	if((length < 0) || (length > (int)(sizeof(number_buffer) - 1)))
		return 0;
	output_pointer = ensure(output_buffer, (size_t)length + sizeof(""));
	if(output_pointer == NULL)
		return 0;
	for(i = 0; i < ((size_t)length); i++)
	{
		if(number_buffer[i] == decimal_point)
		{
			output_pointer[i] = '.';
			continue;
		}
		output_pointer[i] = number_buffer[i];
	}
	output_pointer[i] = '\0';
	output_buffer->offset += (size_t)length;
	return 1;
}

static unsigned parse_hex4(const unsigned char *const input)
{
	unsigned int h = 0;
	size_t i = 0;

	for(i = 0; i < 4; i++)
	{
		if((input[i] >= '0') && (input[i] <= '9'))
			h += (unsigned int)input[i] - '0';
		else if((input[i] >= 'A') && (input[i] <= 'F'))
			h += (unsigned int)10 + input[i] - 'A';
		else if((input[i] >= 'a') && (input[i] <= 'f'))
			h += (unsigned int)10 + input[i] - 'a';
		else
			return 0;
		if(i < 3)
			h = h << 4;
	}
	return h;
}

static unsigned char utf16_literal_to_utf8(const unsigned char *const input_pointer, const unsigned char *const input_end, unsigned char **output_pointer)
{
	long unsigned int codepoint = 0;
	unsigned int first_code = 0;
	const unsigned char *first_sequence = input_pointer;
	unsigned char utf8_length = 0;
	unsigned char utf8_position = 0;
	unsigned char sequence_length = 0;
	unsigned char first_byte_mark = 0;

	if((input_end - first_sequence) < 6)
		goto fail;
	first_code = parse_hex4(first_sequence + 2);
	if(((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
		goto fail;
	if((first_code >= 0xD800) && (first_code <= 0xDBFF))
	{
		const unsigned char *second_sequence = first_sequence + 6;
		unsigned int second_code = 0;
		sequence_length = 12;
		if((input_end - second_sequence) < 6)
			goto fail;
		if((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
			goto fail;
		second_code = parse_hex4(second_sequence + 2);
		if((second_code < 0xDC00) || (second_code > 0xDFFF))
			goto fail;
		codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
	}
	else
	{
		sequence_length = 6;
		codepoint = first_code;
	}
	if(codepoint < 0x80)
	{
		utf8_length = 1;
	}
	else if(codepoint < 0x800)
	{
		utf8_length = 2;
		first_byte_mark = 0xC0;
	}
	else if(codepoint < 0x10000)
	{
		utf8_length = 3;
		first_byte_mark = 0xE0;
	}
	else if(codepoint <= 0x10FFFF)
	{
		utf8_length = 4;
		first_byte_mark = 0xF0;
	}
	else
		goto fail;

	for(utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
	{
		(*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
		codepoint >>= 6;
	}
	if(utf8_length > 1)
		(*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xff);
	else
		(*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
	*output_pointer += utf8_length;
	return sequence_length;
fail:
	return 0;
}

static int parse_string(struct cjson_t * item, struct parse_buffer_t * input_buffer)
{
	const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1;
	const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;
	unsigned char *output_pointer = NULL;
	unsigned char *output = NULL;

	if(buffer_at_offset(input_buffer)[0] != '\"')
	{
		goto fail;
	}
	{
		size_t allocation_length = 0;
		size_t skipped_bytes = 0;
		while(((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
		{
			if(input_end[0] == '\\')
			{
				if((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
					goto fail;
				skipped_bytes++;
				input_end++;
			}
			input_end++;
		}
		if(((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
			goto fail;
		allocation_length = (size_t)(input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
		output = (unsigned char *)xos_mem_malloc(allocation_length + sizeof(""));
		if(output == NULL)
			goto fail;
	}
	output_pointer = output;
	while(input_pointer < input_end)
	{
		if(*input_pointer != '\\')
			*output_pointer++ = *input_pointer++;
		else
		{
			unsigned char sequence_length = 2;
			if((input_end - input_pointer) < 1)
				goto fail;
			switch(input_pointer[1])
			{
			case 'b':
				*output_pointer++ = '\b';
				break;
			case 'f':
				*output_pointer++ = '\f';
				break;
			case 'n':
				*output_pointer++ = '\n';
				break;
			case 'r':
				*output_pointer++ = '\r';
				break;
			case 't':
				*output_pointer++ = '\t';
				break;
			case '\"':
			case '\\':
			case '/':
				*output_pointer++ = input_pointer[1];
				break;

			case 'u':
				sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
				if(sequence_length == 0)
					goto fail;
				break;

			default:
				goto fail;
			}
			input_pointer += sequence_length;
		}
	}
	*output_pointer = '\0';
	item->type = CJSON_STRING;
	item->valuestring = (char*)output;
	input_buffer->offset = (size_t)(input_end - input_buffer->content);
	input_buffer->offset++;
	return 1;
fail:
	if(output != NULL)
	{
		xos_mem_free(output);
		output = NULL;
	}
	if(input_pointer != NULL)
		input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
	return 0;
}

static int print_string_ptr(const unsigned char * input, struct printbuffer_t * output_buffer)
{
	const unsigned char * input_pointer = NULL;
	unsigned char * output = NULL;
	unsigned char * output_pointer = NULL;
	size_t output_length = 0;
	size_t escape_characters = 0;

	if(output_buffer == NULL)
		return 0;
	if(input == NULL)
	{
		output = ensure(output_buffer, sizeof("\"\""));
		if(output == NULL)
			return 0;
		xos_strcpy((char *)output, "\"\"");
		return 1;
	}

	for(input_pointer = input; *input_pointer; input_pointer++)
	{
		switch(*input_pointer)
		{
		case '\"':
		case '\\':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			escape_characters++;
			break;
		default:
			if(*input_pointer < 32)
				escape_characters += 5;
			break;
		}
	}
	output_length = (size_t)(input_pointer - input) + escape_characters;

	output = ensure(output_buffer, output_length + sizeof("\"\""));
	if(output == NULL)
		return 0;

	if(escape_characters == 0)
	{
		output[0] = '\"';
		xos_memcpy(output + 1, input, output_length);
		output[output_length + 1] = '\"';
		output[output_length + 2] = '\0';
		return 1;
	}

	output[0] = '\"';
	output_pointer = output + 1;
	for(input_pointer = input; *input_pointer != '\0'; (void)input_pointer++, output_pointer++)
	{
		if((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\'))
			*output_pointer = *input_pointer;
		else
		{
			*output_pointer++ = '\\';
			switch(*input_pointer)
			{
			case '\\':
				*output_pointer = '\\';
				break;
			case '\"':
				*output_pointer = '\"';
				break;
			case '\b':
				*output_pointer = 'b';
				break;
			case '\f':
				*output_pointer = 'f';
				break;
			case '\n':
				*output_pointer = 'n';
				break;
			case '\r':
				*output_pointer = 'r';
				break;
			case '\t':
				*output_pointer = 't';
				break;
			default:
				xos_sprintf((char *)output_pointer, "u%04x", *input_pointer);
				output_pointer += 4;
				break;
			}
		}
	}
	output[output_length + 1] = '\"';
	output[output_length + 2] = '\0';

	return 1;
}

static int print_string(const struct cjson_t * item, struct printbuffer_t * p)
{
	return print_string_ptr((unsigned char*)item->valuestring, p);
}

static int parse_value(struct cjson_t * item, struct parse_buffer_t * input_buffer);
static int print_value(const struct cjson_t * item, struct printbuffer_t * output_buffer);
static int parse_array(struct cjson_t * item, struct parse_buffer_t * input_buffer);
static int print_array(const struct cjson_t * item, struct printbuffer_t * output_buffer);
static int parse_object(struct cjson_t * item, struct parse_buffer_t * input_buffer);
static int print_object(const struct cjson_t * item, struct printbuffer_t * output_buffer);

static struct parse_buffer_t * buffer_skip_whitespace(struct parse_buffer_t * buffer)
{
	if((buffer == NULL) || (buffer->content == NULL))
		return NULL;
	if(cannot_access_at_index(buffer, 0))
		return buffer;
	while(can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
		buffer->offset++;
	if(buffer->offset == buffer->length)
		buffer->offset--;
	return buffer;
}

static struct parse_buffer_t * skip_utf8_bom(struct parse_buffer_t * buffer)
{
	if((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
		return NULL;
	if(can_access_at_index(buffer, 4) && (xos_strncmp((const char*)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
		buffer->offset += 3;
	return buffer;
}

struct cjson_t * cjson_parse_with_opts(const char * value, const char ** return_parse_end, int require_null_terminated)
{
	size_t buffer_length;
	if(NULL == value)
		return NULL;
	buffer_length = xos_strlen(value) + sizeof("");
	return cjson_parse_with_length_opts(value, buffer_length, return_parse_end, require_null_terminated);
}

struct cjson_t * cjson_parse_with_length_opts(const char * value, size_t buffer_length, const char ** return_parse_end, int require_null_terminated)
{
	struct parse_buffer_t buffer = { 0, 0, 0, 0 };
	struct cjson_t * item = NULL;

	if(value == NULL || 0 == buffer_length)
		goto fail;
	buffer.content = (const unsigned char*)value;
	buffer.length = buffer_length;
	buffer.offset = 0;

	item = cjson_new_item();
	if(item == NULL)
		goto fail;
	if(!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
		goto fail;
	if(require_null_terminated)
	{
		buffer_skip_whitespace(&buffer);
		if((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
			goto fail;
	}
	if(return_parse_end)
		*return_parse_end = (const char*)buffer_at_offset(&buffer);
	return item;
fail:
	if(item != NULL)
		cjson_delete(item);
	if(value != NULL)
	{
		unsigned char * local_error_json = (unsigned char *)value;
		size_t local_error_position = 0;
		if(buffer.offset < buffer.length)
			local_error_position = buffer.offset;
		else if(buffer.length > 0)
			local_error_position = buffer.length - 1;
		if(return_parse_end != NULL)
			*return_parse_end = (const char*)local_error_json + local_error_position;
	}
	return NULL;
}

struct cjson_t * cjson_parse(const char * value)
{
	return cjson_parse_with_opts(value, 0, 0);
}

struct cjson_t * cjson_parse_with_length(const char * value, size_t buffer_length)
{
	return cjson_parse_with_length_opts(value, buffer_length, 0, 0);
}

static unsigned char * print(const struct cjson_t * item, int format)
{
	static const size_t default_buffer_size = 256;
	struct printbuffer_t buffer[1];
	unsigned char * printed = NULL;

	xos_memset(buffer, 0, sizeof(buffer));
	buffer->buffer = (unsigned char *)xos_mem_malloc(default_buffer_size);
	buffer->length = default_buffer_size;
	buffer->format = format;
	if(buffer->buffer == NULL)
		goto fail;
	if(!print_value(item, buffer))
		goto fail;
	update_offset(buffer);

	printed = (unsigned char *)xos_mem_realloc(buffer->buffer, buffer->offset + 1);
	if(printed == NULL)
		goto fail;
	buffer->buffer = NULL;
	return printed;
fail:
	if(buffer->buffer != NULL)
	{
		xos_mem_free(buffer->buffer);
		buffer->buffer = NULL;
	}
	if(printed != NULL)
	{
		xos_mem_free(printed);
		printed = NULL;
	}
	return NULL;
}

char * cjson_print(const struct cjson_t * item)
{
	return (char *)print(item, 1);
}

char * cjson_print_unformatted(const struct cjson_t * item)
{
	return (char *)print(item, 0);
}

char * cjson_print_buffered(const struct cjson_t * item, int prebuffer, int fmt)
{
	struct printbuffer_t p = { 0, 0, 0, 0, 0, 0 };

	if(prebuffer < 0)
		return NULL;
	p.buffer = (unsigned char *)xos_mem_malloc((size_t)prebuffer);
	if(!p.buffer)
		return NULL;
	p.length = (size_t)prebuffer;
	p.offset = 0;
	p.noalloc = 0;
	p.format = fmt;
	if(!print_value(item, &p))
	{
		xos_mem_free(p.buffer);
		p.buffer = NULL;
		return NULL;
	}
	return (char *)p.buffer;
}

int cjson_print_preallocated(struct cjson_t * item, char * buffer, const int length, const int format)
{
	struct printbuffer_t p = { 0, 0, 0, 0, 0, 0 };

	if((length < 0) || (buffer == NULL))
		return 0;
	p.buffer = (unsigned char*)buffer;
	p.length = (size_t)length;
	p.offset = 0;
	p.noalloc = 1;
	p.format = format;
	return print_value(item, &p);
}

static int parse_value(struct cjson_t * item, struct parse_buffer_t * input_buffer)
{
	if((input_buffer == NULL) || (input_buffer->content == NULL))
		return 0;
	if(can_read(input_buffer, 4) && (xos_strncmp((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
	{
		item->type = CJSON_NULL;
		input_buffer->offset += 4;
		return 1;
	}
	if(can_read(input_buffer, 5) && (xos_strncmp((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
	{
		item->type = CJSON_FALSE;
		input_buffer->offset += 5;
		return 1;
	}
	if(can_read(input_buffer, 4) && (xos_strncmp((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
	{
		item->type = CJSON_TRUE;
		item->valueint = 1;
		input_buffer->offset += 4;
		return 1;
	}
	if(can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
		return parse_string(item, input_buffer);
	if(can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
		return parse_number(item, input_buffer);
	if(can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
		return parse_array(item, input_buffer);
	if(can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
		return parse_object(item, input_buffer);
	return 0;
}

static int print_value(const struct cjson_t * item, struct printbuffer_t * output_buffer)
{
	unsigned char * output = NULL;

	if((item == NULL) || (output_buffer == NULL))
		return 0;
	switch((item->type) & 0xff)
	{
	case CJSON_NULL:
		output = ensure(output_buffer, 5);
		if(output == NULL)
			return 0;
		xos_strcpy((char*)output, "null");
		return 1;

	case CJSON_FALSE:
		output = ensure(output_buffer, 6);
		if(output == NULL)
			return 0;
		xos_strcpy((char*)output, "false");
		return 1;

	case CJSON_TRUE:
		output = ensure(output_buffer, 5);
		if(output == NULL)
			return 0;
		xos_strcpy((char *)output, "true");
		return 1;

	case CJSON_NUMBER:
		return print_number(item, output_buffer);

	case CJSON_RAW:
	{
		size_t raw_length = 0;
		if(item->valuestring == NULL)
			return 0;
		raw_length = xos_strlen(item->valuestring) + sizeof("");
		output = ensure(output_buffer, raw_length);
		if(output == NULL)
			return 0;
		xos_memcpy(output, item->valuestring, raw_length);
		return 1;
	}

	case CJSON_STRING:
		return print_string(item, output_buffer);

	case CJSON_ARRAY:
		return print_array(item, output_buffer);

	case CJSON_OBJECT:
		return print_object(item, output_buffer);

	default:
		return 0;
	}
}

static int parse_array(struct cjson_t * item, struct parse_buffer_t * input_buffer)
{
	struct cjson_t *head = NULL;
	struct cjson_t *current_item = NULL;

	if(input_buffer->depth >= CJSON_NESTING_LIMIT)
		return 0;
	input_buffer->depth++;

	if(buffer_at_offset(input_buffer)[0] != '[')
		goto fail;

	input_buffer->offset++;
	buffer_skip_whitespace(input_buffer);
	if(can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
		goto success;

	if(cannot_access_at_index(input_buffer, 0))
	{
		input_buffer->offset--;
		goto fail;
	}

	input_buffer->offset--;
	do
	{
		struct cjson_t * new_item = cjson_new_item();
		if(new_item == NULL)
			goto fail;
		if(head == NULL)
			current_item = head = new_item;
		else
		{
			current_item->next = new_item;
			new_item->prev = current_item;
			current_item = new_item;
		}
		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if(!parse_value(current_item, input_buffer))
			goto fail;
		buffer_skip_whitespace(input_buffer);
	} while(can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

	if(cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
		goto fail;
success:
	input_buffer->depth--;

	if(head != NULL)
		head->prev = current_item;
	item->type = CJSON_ARRAY;
	item->child = head;
	input_buffer->offset++;
	return 1;

fail:
	if(head != NULL)
		cjson_delete(head);
	return 0;
}

static int print_array(const struct cjson_t * item, struct printbuffer_t * output_buffer)
{
	unsigned char * output_pointer = NULL;
	size_t length = 0;
	struct cjson_t * current_element = item->child;

	if(output_buffer == NULL)
		return 0;
	if(output_buffer->depth >= CJSON_NESTING_LIMIT)
		return 0;
	output_pointer = ensure(output_buffer, 1);
	if(output_pointer == NULL)
		return 0;

	*output_pointer = '[';
	output_buffer->offset++;
	output_buffer->depth++;

	while(current_element != NULL)
	{
		if(!print_value(current_element, output_buffer))
			return 0;
		update_offset(output_buffer);
		if(current_element->next)
		{
			length = (size_t)(output_buffer->format ? 2 : 1);
			output_pointer = ensure(output_buffer, length + 1);
			if(output_pointer == NULL)
				return 0;
			*output_pointer++ = ',';
			if(output_buffer->format)
				*output_pointer++ = ' ';
			*output_pointer = '\0';
			output_buffer->offset += length;
		}
		current_element = current_element->next;
	}

	output_pointer = ensure(output_buffer, 2);
	if(output_pointer == NULL)
		return 0;
	*output_pointer++ = ']';
	*output_pointer = '\0';
	output_buffer->depth--;

	return 1;
}

static int parse_object(struct cjson_t * item, struct parse_buffer_t * input_buffer)
{
	struct cjson_t * head = NULL;
	struct cjson_t * current_item = NULL;

	if(input_buffer->depth >= CJSON_NESTING_LIMIT)
		return 0;
	input_buffer->depth++;

	if(cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
		goto fail;

	input_buffer->offset++;
	buffer_skip_whitespace(input_buffer);
	if(can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
		goto success;

	if(cannot_access_at_index(input_buffer, 0))
	{
		input_buffer->offset--;
		goto fail;
	}

	input_buffer->offset--;
	do
	{
		struct cjson_t * new_item = cjson_new_item();
		if(new_item == NULL)
			goto fail;

		if(head == NULL)
			current_item = head = new_item;
		else
		{
			current_item->next = new_item;
			new_item->prev = current_item;
			current_item = new_item;
		}

		if(cannot_access_at_index(input_buffer, 1))
			goto fail;

		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if(!parse_string(current_item, input_buffer))
			goto fail;
		buffer_skip_whitespace(input_buffer);

		current_item->string = current_item->valuestring;
		current_item->valuestring = NULL;

		if(cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
			goto fail;

		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if(!parse_value(current_item, input_buffer))
			goto fail;
		buffer_skip_whitespace(input_buffer);
	} while(can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

	if(cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
		goto fail;

success:
	input_buffer->depth--;

	if(head != NULL)
		head->prev = current_item;
	item->type = CJSON_OBJECT;
	item->child = head;
	input_buffer->offset++;
	return 1;

fail:
	if(head != NULL)
		cjson_delete(head);
	return 0;
}

static int print_object(const struct cjson_t * item, struct printbuffer_t * output_buffer)
{
	unsigned char * output_pointer = NULL;
	size_t length = 0;
	struct cjson_t * current_item = item->child;

	if(output_buffer == NULL)
		return 0;
	if(output_buffer->depth >= CJSON_NESTING_LIMIT)
		return 0;
	length = (size_t)(output_buffer->format ? 2 : 1);
	output_pointer = ensure(output_buffer, length + 1);
	if(output_pointer == NULL)
		return 0;

	*output_pointer++ = '{';
	output_buffer->depth++;
	if(output_buffer->format)
		*output_pointer++ = '\n';
	output_buffer->offset += length;

	while(current_item)
	{
		if(output_buffer->format)
		{
			size_t i;
			output_pointer = ensure(output_buffer, output_buffer->depth);
			if(output_pointer == NULL)
				return 0;
			for(i = 0; i < output_buffer->depth; i++)
				*output_pointer++ = '\t';
			output_buffer->offset += output_buffer->depth;
		}

		if(!print_string_ptr((unsigned char*)current_item->string, output_buffer))
			return 0;
		update_offset(output_buffer);

		length = (size_t)(output_buffer->format ? 2 : 1);
		output_pointer = ensure(output_buffer, length);
		if(output_pointer == NULL)
			return 0;
		*output_pointer++ = ':';
		if(output_buffer->format)
			*output_pointer++ = '\t';
		output_buffer->offset += length;

		if(!print_value(current_item, output_buffer))
			return 0;
		update_offset(output_buffer);

		length = ((size_t)(output_buffer->format ? 1 : 0) + (size_t)(current_item->next ? 1 : 0));
		output_pointer = ensure(output_buffer, length + 1);
		if(output_pointer == NULL)
			return 0;
		if(current_item->next)
			*output_pointer++ = ',';
		if(output_buffer->format)
			*output_pointer++ = '\n';
		*output_pointer = '\0';
		output_buffer->offset += length;
		current_item = current_item->next;
	}

	output_pointer = ensure(output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
	if(output_pointer == NULL)
		return 0;
	if(output_buffer->format)
	{
		size_t i;
		for(i = 0; i < (output_buffer->depth - 1); i++)
			*output_pointer++ = '\t';
	}
	*output_pointer++ = '}';
	*output_pointer = '\0';
	output_buffer->depth--;

	return 1;
}

int cjson_get_array_size(const struct cjson_t * array)
{
	struct cjson_t *child = NULL;
	size_t size = 0;

	if(array == NULL)
		return 0;
	child = array->child;
	while(child != NULL)
	{
		size++;
		child = child->next;
	}
	return (int)size;
}

static struct cjson_t * get_array_item(const struct cjson_t * array, size_t index)
{
	struct cjson_t * current_child = NULL;

	if(array == NULL)
		return NULL;
	current_child = array->child;
	while((current_child != NULL) && (index > 0))
	{
		index--;
		current_child = current_child->next;
	}
	return current_child;
}

struct cjson_t * cjson_get_array_item(const struct cjson_t * array, int index)
{
	if(index < 0)
		return NULL;
	return get_array_item(array, (size_t)index);
}

static struct cjson_t * get_object_item(const struct cjson_t *const object, const char *const name, const int case_sensitive)
{
	struct cjson_t *current_element = NULL;

	if((object == NULL) || (name == NULL))
		return NULL;
	current_element = object->child;
	if(case_sensitive)
	{
		while((current_element != NULL) && (current_element->string != NULL) && (xos_strcmp(name, current_element->string) != 0))
			current_element = current_element->next;
	}
	else
	{
		while((current_element != NULL) && (case_insensitive_strcmp((const unsigned char*)name, (const unsigned char*)(current_element->string)) != 0))
			current_element = current_element->next;
	}
	if((current_element == NULL) || (current_element->string == NULL))
		return NULL;
	return current_element;
}

struct cjson_t * cjson_get_object_item(const struct cjson_t * object, const char * string)
{
	return get_object_item(object, string, 0);
}

struct cjson_t * cjson_get_object_item_case_sensitive(const struct cjson_t * object, const char * string)
{
	return get_object_item(object, string, 1);
}

int cjson_has_object_item(const struct cjson_t * object, const char * string)
{
	return cjson_get_object_item(object, string) ? 1 : 0;
}

static void suffix_object(struct cjson_t * prev, struct cjson_t * item)
{
	prev->next = item;
	item->prev = prev;
}

static struct cjson_t * create_reference(const struct cjson_t * item)
{
	struct cjson_t * reference = NULL;
	if(item == NULL)
		return NULL;
	reference = cjson_new_item();
	if(reference == NULL)
		return NULL;
	xos_memcpy(reference, item, sizeof(struct cjson_t));
	reference->string = NULL;
	reference->type |= CJSON_ISREFERENCE;
	reference->next = reference->prev = NULL;
	return reference;
}

static int add_item_to_array(struct cjson_t *array, struct cjson_t *item)
{
	struct cjson_t * child = NULL;

	if((item == NULL) || (array == NULL) || (array == item))
		return 0;
	child = array->child;
	if(child == NULL)
	{
		array->child = item;
		item->prev = item;
		item->next = NULL;
	}
	else
	{
		if(child->prev)
		{
			suffix_object(child->prev, item);
			array->child->prev = item;
		}
	}
	return 1;
}

static int add_item_to_object(struct cjson_t * object, const char * string, struct cjson_t * item, const int constant_key)
{
	char * new_key = NULL;
	int new_type = CJSON_INVALID;

	if((object == NULL) || (string == NULL) || (item == NULL) || (object == item))
		return 0;
	if(constant_key)
	{
		new_key = (char *)string;
		new_type = item->type | CJSON_STRINGISCONST;
	}
	else
	{
		new_key = (char *)cjson_strdup((const unsigned char *)string);
		if(new_key == NULL)
			return 0;
		new_type = item->type & ~CJSON_STRINGISCONST;
	}
	if(!(item->type & CJSON_STRINGISCONST) && (item->string != NULL))
		xos_mem_free(item->string);
	item->string = new_key;
	item->type = new_type;

	return add_item_to_array(object, item);
}

int cjson_is_invalid(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_INVALID;
}

int cjson_is_false(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_FALSE;
}

int cjson_is_true(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_TRUE;
}

int cjson_is_bool(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & (CJSON_TRUE | CJSON_FALSE)) != 0;
}
int cjson_is_null(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_NULL;
}

int cjson_is_number(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_NUMBER;
}

int cjson_is_string(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_STRING;
}

int cjson_is_array(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_ARRAY;
}

int cjson_is_object(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_OBJECT;
}

int cjson_is_raw(const struct cjson_t * item)
{
	if(item == NULL)
		return 0;
	return (item->type & 0xff) == CJSON_RAW;
}

struct cjson_t * cjson_create_null(void)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
		item->type = CJSON_NULL;
	return item;
}

struct cjson_t * cjson_create_true(void)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
		item->type = CJSON_TRUE;
	return item;
}

struct cjson_t * cjson_create_false(void)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
		item->type = CJSON_FALSE;
	return item;
}

struct cjson_t * cjson_create_bool(int value)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
		item->type = value ? CJSON_TRUE : CJSON_FALSE;
	return item;
}

struct cjson_t * cjson_create_number(double num)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
	{
		item->type = CJSON_NUMBER;
		item->valuedouble = num;
		if(num >= INT_MAX)
			item->valueint = INT_MAX;
		else if(num <= (double)INT_MIN)
			item->valueint = INT_MIN;
		else
			item->valueint = (int)num;
	}
	return item;
}

struct cjson_t * cjson_create_string(const char * str)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
	{
		item->type = CJSON_STRING;
		item->valuestring = (char *)cjson_strdup((const unsigned char *)str);
		if(!item->valuestring)
		{
			cjson_delete(item);
			return NULL;
		}
	}
	return item;
}

struct cjson_t * cjson_create_raw(const char * raw)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
	{
		item->type = CJSON_RAW;
		item->valuestring = (char *)cjson_strdup((const unsigned char *)raw);
		if(!item->valuestring)
		{
			cjson_delete(item);
			return NULL;
		}
	}
	return item;
}

struct cjson_t * cjson_create_array(void)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
		item->type = CJSON_ARRAY;
	return item;
}

struct cjson_t * cjson_create_object(void)
{
	struct cjson_t * item = cjson_new_item();
	if(item)
		item->type = CJSON_OBJECT;
	return item;
}

struct cjson_t * cjson_create_string_reference(const char * str)
{
	struct cjson_t * item = cjson_new_item();
	if(item != NULL)
	{
		item->type = CJSON_STRING | CJSON_ISREFERENCE;
		item->valuestring = (char *)str;
	}
	return item;
}

struct cjson_t * cjson_create_array_reference(struct cjson_t * child)
{
	struct cjson_t * item = cjson_new_item();
	if(item != NULL)
	{
		item->type = CJSON_ARRAY | CJSON_ISREFERENCE;
		item->child = (struct cjson_t *)child;
	}
	return item;
}

struct cjson_t * cjson_create_object_reference(struct cjson_t * child)
{
	struct cjson_t * item = cjson_new_item();
	if(item != NULL)
	{
		item->type = CJSON_OBJECT | CJSON_ISREFERENCE;
		item->child = child;
	}
	return item;
}

struct cjson_t * cjson_create_int_array(const int * numbers, int count)
{
	struct cjson_t * n = NULL;
	struct cjson_t * p = NULL;
	struct cjson_t * a = NULL;
	size_t i = 0;

	if((count < 0) || (numbers == NULL))
		return NULL;
	a = cjson_create_array();
	for(i = 0; a && (i < (size_t)count); i++)
	{
		n = cjson_create_number(numbers[i]);
		if(!n)
		{
			cjson_delete(a);
			return NULL;
		}
		if(!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	if(a && a->child)
		a->child->prev = n;
	return a;
}

struct cjson_t * cjson_create_float_array(const float * numbers, int count)
{
	struct cjson_t * n = NULL;
	struct cjson_t * p = NULL;
	struct cjson_t * a = NULL;
	size_t i = 0;

	if((count < 0) || (numbers == NULL))
		return NULL;
	a = cjson_create_array();
	for(i = 0; a && (i < (size_t)count); i++)
	{
		n = cjson_create_number((double)numbers[i]);
		if(!n)
		{
			cjson_delete(a);
			return NULL;
		}
		if(!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	if(a && a->child)
		a->child->prev = n;
	return a;
}

struct cjson_t * cjson_create_double_array(const double * numbers, int count)
{
	struct cjson_t * n = NULL;
	struct cjson_t * p = NULL;
	struct cjson_t * a = NULL;
	size_t i = 0;

	if((count < 0) || (numbers == NULL))
		return NULL;
	a = cjson_create_array();
	for(i = 0; a && (i < (size_t)count); i++)
	{
		n = cjson_create_number(numbers[i]);
		if(!n)
		{
			cjson_delete(a);
			return NULL;
		}
		if(!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	if(a && a->child)
		a->child->prev = n;
	return a;
}

struct cjson_t * cjson_create_string_array(const char ** strings, int count)
{
	struct cjson_t * n = NULL;
	struct cjson_t * p = NULL;
	struct cjson_t * a = NULL;
	size_t i = 0;

	if((count < 0) || (strings == NULL))
		return NULL;
	a = cjson_create_array();
	for(i = 0; a && (i < (size_t)count); i++)
	{
		n = cjson_create_string(strings[i]);
		if(!n)
		{
			cjson_delete(a);
			return NULL;
		}
		if(!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	if(a && a->child)
		a->child->prev = n;
	return a;
}

struct cjson_t * cjson_add_null_to_object(struct cjson_t * object, const char * name)
{
	struct cjson_t * null = cjson_create_null();
	if(add_item_to_object(object, name, null, 0))
		return null;
	cjson_delete(null);
	return NULL;
}

struct cjson_t * cjson_add_true_to_object(struct cjson_t * object, const char * name)
{
	struct cjson_t * true_item = cjson_create_true();
	if(add_item_to_object(object, name, true_item, 0))
		return true_item;
	cjson_delete(true_item);
	return NULL;
}

struct cjson_t * cjson_add_false_to_object(struct cjson_t * object, const char * name)
{
	struct cjson_t * false_item = cjson_create_false();
	if(add_item_to_object(object, name, false_item, 0))
		return false_item;
	cjson_delete(false_item);
	return NULL;
}

struct cjson_t * cjson_add_bool_to_object(struct cjson_t * object, const char * name, const int boolean)
{
	struct cjson_t * bool_item = cjson_create_bool(boolean);
	if(add_item_to_object(object, name, bool_item, 0))
		return bool_item;
	cjson_delete(bool_item);
	return NULL;
}

struct cjson_t * cjson_add_number_to_object(struct cjson_t * object, const char * name, const double number)
{
	struct cjson_t * number_item = cjson_create_number(number);
	if(add_item_to_object(object, name, number_item, 0))
		return number_item;
	cjson_delete(number_item);
	return NULL;
}

struct cjson_t * cjson_add_string_to_object(struct cjson_t * object, const char * name, const char * string)
{
	struct cjson_t * string_item = cjson_create_string(string);
	if(add_item_to_object(object, name, string_item, 0))
		return string_item;
	cjson_delete(string_item);
	return NULL;
}

struct cjson_t * cjson_add_raw_to_object(struct cjson_t * object, const char * name, const char * raw)
{
	struct cjson_t * raw_item = cjson_create_raw(raw);
	if(add_item_to_object(object, name, raw_item, 0))
		return raw_item;
	cjson_delete(raw_item);
	return NULL;
}

struct cjson_t * cjson_add_object_to_object(struct cjson_t * object, const char * name)
{
	struct cjson_t * object_item = cjson_create_object();
	if(add_item_to_object(object, name, object_item, 0))
		return object_item;
	cjson_delete(object_item);
	return NULL;
}

struct cjson_t * cjson_add_array_to_object(struct cjson_t * object, const char * name)
{
	struct cjson_t * array = cjson_create_array();
	if(add_item_to_object(object, name, array, 0))
		return array;
	cjson_delete(array);
	return NULL;
}

int cjson_add_item_to_array(struct cjson_t * array, struct cjson_t * item)
{
	return add_item_to_array(array, item);
}

int cjson_add_item_to_object(struct cjson_t * object, const char * string, struct cjson_t * item)
{
	return add_item_to_object(object, string, item, 0);
}

int cjson_add_item_to_object_cs(struct cjson_t * object, const char * string, struct cjson_t * item)
{
	return add_item_to_object(object, string, item, 1);
}

int cjson_add_item_reference_to_array(struct cjson_t * array, struct cjson_t * item)
{
	if(array == NULL)
		return 0;
	return add_item_to_array(array, create_reference(item));
}

int cjson_add_item_reference_to_object(struct cjson_t * object, const char * string, struct cjson_t * item)
{
	if((object == NULL) || (string == NULL))
		return 0;
	return add_item_to_object(object, string, create_reference(item), 0);
}

struct cjson_t * cjson_detach_item_via_pointer(struct cjson_t * parent, struct cjson_t * item)
{
	if((parent == NULL) || (item == NULL) || (item != parent->child && item->prev == NULL))
		return NULL;
	if(item != parent->child)
		item->prev->next = item->next;
	if(item->next != NULL)
		item->next->prev = item->prev;
	if(item == parent->child)
		parent->child = item->next;
	else if(item->next == NULL)
		parent->child->prev = item->prev;
	item->prev = NULL;
	item->next = NULL;
	return item;
}

struct cjson_t * cjson_detach_item_from_array(struct cjson_t * array, int which)
{
	if(which < 0)
		return NULL;
	return cjson_detach_item_via_pointer(array, get_array_item(array, (size_t)which));
}

struct cjson_t * cjson_detach_item_from_object(struct cjson_t * object, const char * string)
{
	struct cjson_t * to_detach = cjson_get_object_item(object, string);
	return cjson_detach_item_via_pointer(object, to_detach);
}

struct cjson_t * cjson_detach_item_from_object_case_sensitive(struct cjson_t * object, const char * string)
{
	struct cjson_t * to_detach = cjson_get_object_item_case_sensitive(object, string);
	return cjson_detach_item_via_pointer(object, to_detach);
}

void cjson_delete_item_from_array(struct cjson_t * array, int which)
{
	cjson_delete(cjson_detach_item_from_array(array, which));
}

void cjson_delete_item_from_object(struct cjson_t * object, const char * string)
{
	cjson_delete(cjson_detach_item_from_object(object, string));
}

void cjson_delete_item_from_object_case_sensitive(struct cjson_t * object, const char * string)
{
	cjson_delete(cjson_detach_item_from_object_case_sensitive(object, string));
}

int cjson_insert_item_in_array(struct cjson_t * array, int which, struct cjson_t * newitem)
{
	struct cjson_t * after_inserted = NULL;

	if(which < 0 || newitem == NULL)
		return 0;
	after_inserted = get_array_item(array, (size_t)which);
	if(after_inserted == NULL)
		return add_item_to_array(array, newitem);
	if(after_inserted != array->child && after_inserted->prev == NULL)
		return 0;
	newitem->next = after_inserted;
	newitem->prev = after_inserted->prev;
	after_inserted->prev = newitem;
	if(after_inserted == array->child)
		array->child = newitem;
	else
		newitem->prev->next = newitem;
	return 1;
}

int cjson_replace_item_via_pointer(struct cjson_t * parent, struct cjson_t * item, struct cjson_t * replacement)
{
	if((parent == NULL) || (parent->child == NULL) || (replacement == NULL) || (item == NULL))
		return 0;
	if(replacement == item)
		return 1;
	replacement->next = item->next;
	replacement->prev = item->prev;

	if(replacement->next != NULL)
		replacement->next->prev = replacement;
	if(parent->child == item)
	{
		if(parent->child->prev == parent->child)
			replacement->prev = replacement;
		parent->child = replacement;
	}
	else
	{
		if(replacement->prev != NULL)
			replacement->prev->next = replacement;
		if(replacement->next == NULL)
			parent->child->prev = replacement;
	}
	item->next = NULL;
	item->prev = NULL;
	cjson_delete(item);
	return 1;
}

int cjson_replace_item_in_array(struct cjson_t * array, int which, struct cjson_t * newitem)
{
	if(which < 0)
		return 0;
	return cjson_replace_item_via_pointer(array, get_array_item(array, (size_t)which), newitem);
}

static int replace_item_in_object(struct cjson_t * object, const char *string, struct cjson_t * replacement, int case_sensitive)
{
	if((replacement == NULL) || (string == NULL))
		return 0;
	if(!(replacement->type & CJSON_STRINGISCONST) && (replacement->string != NULL))
	{
		xos_mem_free(replacement->string);
		replacement->string = NULL;
	}
	replacement->string = (char *)cjson_strdup((const unsigned char *)string);
	if(replacement->string == NULL)
		return 0;
	replacement->type &= ~CJSON_STRINGISCONST;
	return cjson_replace_item_via_pointer(object, get_object_item(object, string, case_sensitive), replacement);
}

int cjson_replace_item_in_object(struct cjson_t * object, const char * string, struct cjson_t * newitem)
{
	return replace_item_in_object(object, string, newitem, 0);
}

int cjson_replace_item_in_object_case_sensitive(struct cjson_t * object, const char * string, struct cjson_t * newitem)
{
	return replace_item_in_object(object, string, newitem, 1);
}

static struct cjson_t * cjson_duplicate_rec(const struct cjson_t * item, size_t depth, int recurse)
{
	struct cjson_t * newitem = NULL;
	struct cjson_t * child = NULL;
	struct cjson_t * next = NULL;
	struct cjson_t * newchild = NULL;

	if(!item)
		goto fail;
	newitem = cjson_new_item();
	if(!newitem)
		goto fail;
	newitem->type = item->type & (~CJSON_ISREFERENCE);
	newitem->valueint = item->valueint;
	newitem->valuedouble = item->valuedouble;
	if(item->valuestring)
	{
		newitem->valuestring = (char *)cjson_strdup((const unsigned char *)item->valuestring);
		if(!newitem->valuestring)
			goto fail;
	}
	if(item->string)
	{
		newitem->string = (item->type & CJSON_STRINGISCONST) ? item->string : (char *)cjson_strdup((const unsigned char *)item->string);
		if(!newitem->string)
			goto fail;
	}
	if(!recurse)
		return newitem;
	child = item->child;
	while(child != NULL)
	{
		if(depth >= CJSON_CIRCULAR_LIMIT)
			goto fail;
		newchild = cjson_duplicate_rec(child, depth + 1, 1);
		if(!newchild)
			goto fail;
		if(next != NULL)
		{
			next->next = newchild;
			newchild->prev = next;
			next = newchild;
		}
		else
		{
			newitem->child = newchild;
			next = newchild;
		}
		child = child->next;
	}
	if(newitem && newitem->child)
		newitem->child->prev = newchild;
	return newitem;
fail:
	if(newitem != NULL)
		cjson_delete(newitem);
	return NULL;
}

struct cjson_t * cjson_duplicate(const struct cjson_t * item, int recurse)
{
	return cjson_duplicate_rec(item, 0, recurse);
}

static void skip_oneline_comment(char ** input)
{
	*input += CJSON_STATIC_STRLEN("//");

	for(; (*input)[0] != '\0'; ++(*input))
	{
		if((*input)[0] == '\n')
		{
			*input += CJSON_STATIC_STRLEN("\n");
			return;
		}
	}
}

static void skip_multiline_comment(char **input)
{
	*input += CJSON_STATIC_STRLEN("/*");

	for(; (*input)[0] != '\0'; ++(*input))
	{
		if(((*input)[0] == '*') && ((*input)[1] == '/'))
		{
			*input += CJSON_STATIC_STRLEN("*/");
			return;
		}
	}
}

static void minify_string(char **input, char **output)
{
	(*output)[0] = (*input)[0];
	*input += CJSON_STATIC_STRLEN("\"");
	*output += CJSON_STATIC_STRLEN("\"");

	for(; (*input)[0] != '\0'; (void)++(*input), ++(*output))
	{
		(*output)[0] = (*input)[0];

		if((*input)[0] == '\"')
		{
			(*output)[0] = '\"';
			*input += CJSON_STATIC_STRLEN("\"");
			*output += CJSON_STATIC_STRLEN("\"");
			return;
		}
		else if(((*input)[0] == '\\') && ((*input)[1] == '\"'))
		{
			(*output)[1] = (*input)[1];
			*input += CJSON_STATIC_STRLEN("\"");
			*output += CJSON_STATIC_STRLEN("\"");
		}
	}
}

void cjson_minify(char * json)
{
	char * into = json;

	if(json == NULL)
		return;
	while(json[0] != '\0')
	{
		switch(json[0])
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			json++;
			break;

		case '/':
			if(json[1] == '/')
				skip_oneline_comment(&json);
			else if(json[1] == '*')
				skip_multiline_comment(&json);
			else
				json++;
			break;

		case '\"':
			minify_string(&json, (char**)&into);
			break;

		default:
			into[0] = json[0];
			json++;
			into++;
		}
	}
	*into = '\0';
}

int cjson_compare(const struct cjson_t * a, const struct cjson_t * b, int case_sensitive)
{
	if((a == NULL) || (b == NULL) || ((a->type & 0xff) != (b->type & 0xff)))
		return 0;
	switch(a->type & 0xff)
	{
	case CJSON_FALSE:
	case CJSON_TRUE:
	case CJSON_NULL:
	case CJSON_NUMBER:
	case CJSON_STRING:
	case CJSON_RAW:
	case CJSON_ARRAY:
	case CJSON_OBJECT:
		break;

	default:
		return 0;
	}

	if(a == b)
		return 1;

	switch(a->type & 0xff)
	{
	case CJSON_FALSE:
	case CJSON_TRUE:
	case CJSON_NULL:
		return 1;

	case CJSON_NUMBER:
		if(compare_double(a->valuedouble, b->valuedouble))
			return 1;
		return 0;

	case CJSON_STRING:
	case CJSON_RAW:
		if((a->valuestring == NULL) || (b->valuestring == NULL))
			return 0;
		if(xos_strcmp(a->valuestring, b->valuestring) == 0)
			return 1;
		return 0;

	case CJSON_ARRAY:
	{
		struct cjson_t * a_element = a->child;
		struct cjson_t * b_element = b->child;

		for(; (a_element != NULL) && (b_element != NULL);)
		{
			if(!cjson_compare(a_element, b_element, case_sensitive))
				return 0;
			a_element = a_element->next;
			b_element = b_element->next;
		}
		if(a_element != b_element)
			return 0;
		return 1;
	}

	case CJSON_OBJECT:
	{
		struct cjson_t * a_element = NULL;
		struct cjson_t * b_element = NULL;
		cjson_array_for_each(a_element, a)
		{
			b_element = get_object_item(b, a_element->string, case_sensitive);
			if(b_element == NULL)
				return 0;
			if(!cjson_compare(a_element, b_element, case_sensitive))
				return 0;
		}
		cjson_array_for_each(b_element, b)
		{
			a_element = get_object_item(a, b_element->string, case_sensitive);
			if(a_element == NULL)
				return 0;
			if(!cjson_compare(b_element, a_element, case_sensitive))
				return 0;
		}
		return 1;
	}

	default:
		return 0;
	}
}
