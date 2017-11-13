/*
Copyright (c) 2013-2017
Mahmoud Fayed <msfclipper@yahoo.com>
Hassan Ahmed <hsn@outlook.hu>
*/

#include "ring.h"
#include "ring_pcre.h"

RING_API void ringlib_init ( RingState *pRingState )
{
	ring_vm_funcregister("pcre_match", ring_pcre_match);
}

void ring_pcre_match(void *pPointer)
{
    List *retval, *tmp_list;

    // RING_PI pi;
    // int pi_error;

    pcre2_code *code;
    PCRE2_SPTR pattern;
    int options;
    int err_code;
    PCRE2_SIZE err_offset;
    pcre2_match_data *pcre_md;
    int match;
    PCRE2_SPTR subject;
    PCRE2_SIZE sublen;
    // PCRE2_SIZE start_offset;
    PCRE2_SIZE *ovectorp;

    // PCRE2_SPTR tpl_name;
    // int name_count;
    // int name_size;

    if (RING_API_PARACOUNT != 3) {
        RING_API_ERROR(RING_API_MISS2PARA);
        return;
    }

    if (! RING_API_ISSTRING(1)) {
        RING_API_ERROR("This function excpects the first paratmeter to be a string");
        return;
    }

    if (! RING_API_ISLIST(2)) {
        RING_API_ERROR("This function excpects the second paratmeter to be a list");
        return;
    }

    pattern = (PCRE2_SPTR) RING_API_GETSTRING(1);
    options = ring_pcre2_parse_options(RING_API_GETLIST(2));
    subject = (PCRE2_SPTR) RING_API_GETSTRING(3);
    sublen = strlen(RING_API_GETSTRING(3));

    code = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, options, &err_code, &err_offset, NULL);

    if (code == NULL) {
        RING_API_ERROR("Compilation failed!");
        return;
    }

    pcre_md = pcre2_match_data_create_from_pattern(code, NULL);

    match = pcre2_match(
        code,
        subject,
        sublen,
        0,
        options,
        pcre_md,
        NULL
    );

    if (match < 0) {
        RING_API_ERROR("Match failed!");
        return;
    }

    if (match == PCRE2_ERROR_NOMATCH) {
        RING_API_ERROR("No matches found");
        return;
    }

    ovectorp = pcre2_get_ovector_pointer(pcre_md);

    retval = RING_API_NEWLIST;
    tmp_list = ring_list_newlist_gc(((VM *)pPointer)->pRingState, retval);

    for (int i = 0; i < match; i++) {
        PCRE2_SPTR substr = subject + ovectorp[i * 2];
        int substr_len = ovectorp[(i * 2) + 1] - ovectorp[i * 2];
        ring_list_addstring2(tmp_list, (char* )substr, substr_len);
    }

    RING_API_RETLIST(retval);
}


int ring_pcre2_explain_option(char* option)
{
    switch(*option) {
        case 'x': return PCRE2_EXTENDED; break;
        case 'm': return PCRE2_MULTILINE; break;
        case 'i': return PCRE2_CASELESS; break;
        case 's': return PCRE2_DOTALL; break;
        default:
            // return error
            return 1;
    }
}

int ring_pcre2_parse_options(List *opt_list)
{
    int list_size;

    int tmp_options = 0;

    list_size = ring_list_getsize(opt_list);

    for (int i = 1; i <= list_size; i++) {
        if (ring_list_isstring(opt_list, i)) {
            tmp_options |= ring_pcre2_explain_option(
                ring_list_getstring(opt_list, i)
            );
        } else {
            return 1;
        }
    }

    return tmp_options;
}
