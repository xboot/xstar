#ifndef __CJSON_h__
#define __CJSON_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstar.h>

enum {
	CJSON_INVALID		= (0 << 0),
	CJSON_FALSE			= (1 << 0),
	CJSON_TRUE			= (1 << 1),
	CJSON_NULL			= (1 << 2),
	CJSON_NUMBER		= (1 << 3),
	CJSON_STRING		= (1 << 4),
	CJSON_ARRAY			= (1 << 5),
	CJSON_OBJECT		= (1 << 6),
	CJSON_RAW			= (1 << 7),
	CJSON_ISREFERENCE	= (1 << 8),
	CJSON_STRINGISCONST	= (1 << 9),
};

struct cjson_t {
	struct cjson_t * next;
	struct cjson_t * prev;
	struct cjson_t * child;

	int type;
	char * valuestring;
	int valueint;
	double valuedouble;
	char * string;
};

struct cjson_t * cjson_parse(const char * value);
struct cjson_t * cjson_parse_with_length(const char * value, size_t buffer_length);
struct cjson_t * cjson_parse_with_opts(const char * value, const char ** return_parse_end, int require_null_terminated);
struct cjson_t * cjson_parse_with_length_opts(const char * value, size_t buffer_length, const char ** return_parse_end, int require_null_terminated);
char * cjson_print(const struct cjson_t * item);
char * cjson_print_unformatted(const struct cjson_t * item);
char * cjson_print_buffered(const struct cjson_t * item, int prebuffer, int fmt);
int cjson_print_preallocated(struct cjson_t * item, char * buffer, const int length, const int format);
void cjson_delete(struct cjson_t * item);

int cjson_get_array_size(const struct cjson_t * array);
struct cjson_t * cjson_get_array_item(const struct cjson_t * array, int index);
struct cjson_t * cjson_get_object_item(const struct cjson_t * object, const char * string);
struct cjson_t * cjson_get_object_item_case_sensitive(const struct cjson_t * object, const char * string);
int cjson_has_object_item(const struct cjson_t * object, const char * string);

char * cjson_get_string_value(const struct cjson_t * item);
double cjson_get_number_value(const struct cjson_t * item);

int cjson_is_invalid(const struct cjson_t * item);
int cjson_is_false(const struct cjson_t * item);
int cjson_is_true(const struct cjson_t * item);
int cjson_is_bool(const struct cjson_t * item);
int cjson_is_null(const struct cjson_t * item);
int cjson_is_number(const struct cjson_t * item);
int cjson_is_string(const struct cjson_t * item);
int cjson_is_array(const struct cjson_t * item);
int cjson_is_object(const struct cjson_t * item);
int cjson_is_raw(const struct cjson_t * item);

struct cjson_t * cjson_create_null(void);
struct cjson_t * cjson_create_true(void);
struct cjson_t * cjson_create_false(void);
struct cjson_t * cjson_create_bool(int value);
struct cjson_t * cjson_create_number(double num);
struct cjson_t * cjson_create_string(const char * str);
struct cjson_t * cjson_create_raw(const char * raw);
struct cjson_t * cjson_create_array(void);
struct cjson_t * cjson_create_object(void);

struct cjson_t * cjson_create_string_reference(const char * str);
struct cjson_t * cjson_create_object_reference(struct cjson_t * child);
struct cjson_t * cjson_create_array_reference(struct cjson_t * child);

struct cjson_t * cjson_create_int_array(const int * numbers, int count);
struct cjson_t * cjson_create_float_array(const float * numbers, int count);
struct cjson_t * cjson_create_double_array(const double * numbers, int count);
struct cjson_t * cjson_create_string_array(const char ** strings, int count);

struct cjson_t * cjson_add_null_to_object(struct cjson_t * object, const char * name);
struct cjson_t * cjson_add_true_to_object(struct cjson_t * object, const char * name);
struct cjson_t * cjson_add_false_to_object(struct cjson_t * object, const char * name);
struct cjson_t * cjson_add_bool_to_object(struct cjson_t * object, const char * name, const int boolean);
struct cjson_t * cjson_add_number_to_object(struct cjson_t * object, const char * name, const double number);
struct cjson_t * cjson_add_string_to_object(struct cjson_t * object, const char * name, const char * string);
struct cjson_t * cjson_add_raw_to_object(struct cjson_t * object, const char * name, const char * raw);
struct cjson_t * cjson_add_object_to_object(struct cjson_t * object, const char * name);
struct cjson_t * cjson_add_array_to_object(struct cjson_t * object, const char * name);

int cjson_add_item_to_array(struct cjson_t * array, struct cjson_t * item);
int cjson_add_item_to_object(struct cjson_t * object, const char * string, struct cjson_t * item);
int cjson_add_item_to_object_cs(struct cjson_t * object, const char * string, struct cjson_t * item);
int cjson_add_item_reference_to_array(struct cjson_t * array, struct cjson_t * item);
int cjson_add_item_reference_to_object(struct cjson_t * object, const char * string, struct cjson_t * item);

struct cjson_t * cjson_detach_item_via_pointer(struct cjson_t * parent, struct cjson_t * item);
struct cjson_t * cjson_detach_item_from_array(struct cjson_t * array, int which);
struct cjson_t * cjson_detach_item_from_object(struct cjson_t * object, const char * string);
struct cjson_t * cjson_detach_item_from_object_case_sensitive(struct cjson_t * object, const char * string);

void cjson_delete_item_from_array(struct cjson_t * array, int which);
void cjson_delete_item_from_object(struct cjson_t * object, const char * string);
void cjson_delete_item_from_object_case_sensitive(struct cjson_t * object, const char * string);

int cjson_insert_item_in_array(struct cjson_t * array, int which, struct cjson_t * newitem);
int cjson_replace_item_via_pointer(struct cjson_t * parent, struct cjson_t * item, struct cjson_t * replacement);
int cjson_replace_item_in_array(struct cjson_t * array, int which, struct cjson_t * newitem);
int cjson_replace_item_in_object(struct cjson_t * object, const char * string, struct cjson_t * newitem);
int cjson_replace_item_in_object_case_sensitive(struct cjson_t * object, const char * string, struct cjson_t * newitem);

struct cjson_t * cjson_duplicate(const struct cjson_t * item, int recurse);
int cjson_compare(const struct cjson_t * a, const struct cjson_t * b, int case_sensitive);
void cjson_minify(char * json);

double cjson_set_number_helper(struct cjson_t * object, double number);
char * cjson_set_valuestring(struct cjson_t * object, const char * valuestring);

#define cjson_set_int_value(object, number)		((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
#define cjson_set_number_value(object, number)	((object != NULL) ? cjson_set_number_helper(object, (double)number) : (number))
#define cjson_set_bool_value(object, boolValue)	((object != NULL && ((object)->type & (CJSON_FALSE|CJSON_TRUE))) ? (object)->type=((object)->type &(~(CJSON_FALSE|CJSON_TRUE)))|((boolValue)?CJSON_TRUE:CJSON_FALSE) : CJSON_INVALID)
#define cjson_array_for_each(element, array)	for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

#ifdef __cplusplus
}
#endif

#endif /* __CJSON_h__ */
