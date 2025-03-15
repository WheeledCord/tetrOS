#include "math.h"
#include "colours.h"
#ifndef STRING_UTLS_H
#define STRING_UTLS_H

void itoa(int value, char *str, int base) {
    char *rc = str, *ptr = str, *low;
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }
    int num = value;
    do {
        *ptr++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base];
        num /= base;
    } while (num);
    *ptr = '\0';
    low = rc;
    if (*rc == '-') {
        low++;
    }
    char *high = ptr - 1;
    while (low < high) {
        char temp = *low;
        *low++ = *high;
        *high-- = temp;
    }
}

unsigned int get_str_length(char *string) {
    unsigned int string_length = 0;
    while(*string) {
        string++;
        string_length++;
    }
    for(unsigned int i = 0; i < string_length; i++) {
        string--;
    }
    return string_length;
}

unsigned int get_int_length(int v) {
    unsigned int string_length = 0;
    char *string = "";
    itoa(v,string,10);
    return get_str_length(string);
}

void leading_zero_adder(int v, unsigned int desired_length, char *string) {
    unsigned int len = get_int_length(v);
    if (len < desired_length) {
        for (unsigned int i = 0; i <= desired_length-1; i++) {
            string[i] = '0';
        }
        char *int_string = "";
        itoa(v,int_string,10);
        unsigned int i = 0;
        for (unsigned int ii = desired_length-1; ii > desired_length-len-1; ii--) {
            string[ii] = int_string[i++];
        }
        string[desired_length] = '\0';
    } else {
        itoa(v,string,10);
    }
}

void set_str(char *to, char *from) {
    unsigned int i = 0;
    for (unsigned int ii = 0; ii < get_str_length(from); ii++) {
        to[i++] = from[ii];
    }
    to[i] = '\0';
}

// end is inclusive
void slice_str(char *to, char *string, unsigned int start, unsigned int end) {
    // Handle invalid range
    if (start >= get_str_length(string) || end >= get_str_length(string) || start > end) {
        return;
    }
    char out[end-start+1];
    unsigned int i = 0;
    for (unsigned int ii = start; ii < end+1; ii++) {
        out[i++] = string[ii];
    }
    out[i] = '\0';
    set_str(to,out);
}

void join_str(char *out, char *a, char *b) {
    unsigned int i = 0;
    for (unsigned int ii = 0; ii < get_str_length(a); ii++) {
        out[i++] = a[ii];
    }
    for (unsigned int ii = 0; ii < get_str_length(b); ii++) {
        out[i++] = b[ii];
    }
    out[i] = '\0';
}

unsigned int str_count(char *string, char *sub) {
    unsigned int amount = 0;
    unsigned int sub_i = 0;
    for(unsigned int i = 0; i < get_str_length(string); i++) {
        if (string[i] == sub[sub_i]) {
            sub_i++;
        } else {
            sub_i = 0;
        }
        if (sub_i == get_str_length(sub)) {
            amount++;
            sub_i = 0;
        }
    }
    return amount;
}

unsigned int str_count_c(char *string, char c) {
    unsigned int amount = 0;
    for(unsigned int i = 0; i < get_str_length(string); i++) {
        if (string[i] == c) {
            amount++;
        }
    }
    return amount;
}

int str_find_c(char *string, char c, unsigned int start) {
    unsigned int amount = 0;
    for(unsigned int i = start; i < get_str_length(string); i++) {
        if (string[i] == c) {
            return i;
        }
    }
    return -1;
}

char digit_chars[] = "0123456789";
int parse_int(char *str) {
    int result = 0;
    bool negative = false;
    for (unsigned int i = 0; i < get_str_length(str); i++) {
        char c = str[i];
        if (i == 0 && c == '-') {
            negative = true;
        } else {
            int digit = str_find_c(digit_chars,c,0);
            if (digit != -1) {
                unsigned int exp = get_str_length(str) - (i+1);
                result += digit*power_of(10,exp);
            } else {
                return 0;
            }
        }
    }
    if (negative) {
        result = -1*result;
    }
    return result;
}

void debug_printi(int i,unsigned int y) {
    char *vidmem = (char *)0xB8000;
    char *string = "";
    itoa(i,string,10);
    unsigned int ii = y * 80 * 2;
    while (*string) {
        vidmem[ii++] = *string++;
        vidmem[ii++] = WHITE;
    }
}
void debug_print(char string[],unsigned int y) {
    char *vidmem = (char *)0xB8000;
    unsigned int ii = y * 80 * 2;
    while (*string) {
        vidmem[ii++] = *string++;
        vidmem[ii++] = WHITE;
    }
}

void format_str(char *to, char *string, unsigned int arg_count, ...) {
    char **args = (char **)(&string+1);
    unsigned int max_arg_length = 0;
    for (unsigned int i = 0; i < arg_count; i++) {
        unsigned int len = get_str_length(args[i]);
        if (len > max_arg_length) {
            max_arg_length = len;
        }
    }
    unsigned int len = (floor(str_count_c(string,'%')/2)*max_arg_length)+get_str_length(string);
    char out[len+1];

    unsigned int dp = 0;

    unsigned int i = 0;
    for (unsigned int ii = 0; ii < get_str_length(string); ii++) {
        char c = string[ii];
        if (c == '%') {
            bool valid = true;
            unsigned int closing_index = str_find_c(string,'%',ii+1);
            char index[4];
            slice_str(index,string,ii+1,closing_index-1);
            if (get_str_length(index) == 0) {
                valid = false;
            }
            if (valid) {
                char *arg = args[parse_int(index)];
                dp++;
                for (unsigned int iii = 0; iii < get_str_length(arg); iii++) {
                    out[i++] = arg[iii];
                }
            }
            ii = closing_index + 1;
        } else {
            out[i++] = c;
        }
    }
    out[i] = '\0';
    set_str(to,out);
}

#endif