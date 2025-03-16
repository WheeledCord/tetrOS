#include "math.h"
#include "classes.h"
#ifndef STRING_UTLS_H
#define STRING_UTLS_H

int count_digits(int n) {
    int count = (n <= 0) ? 1 : 0;
    while (n != 0) {
        n /= 10;
        count++;
    }
    return count;
}

int last_n_digits(int num, unsigned int n) {
    int total_digits = count_digits(num);
    
    if (n >= total_digits) {
        return num;
    }

    int divisor = 1;
    for (unsigned int i = 0; i < total_digits - n; i++) {
        divisor *= 10;
    }

    return num % divisor;
}

void itoa(char *to, int value, int base) {
    char *rc = to, *ptr = to, *low;
    if (base < 2 || base > 36) {
        *to = '\0';
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

void leading_zero_adder(char *to, int num, unsigned int desired_length) {
    if (num >= power_of(10,desired_length-1)) {
        itoa(to, num, 10);
    } else {
        char temp[get_str_length(to)];
        itoa(temp, num, 10);
        unsigned int zeros_to_add = desired_length - get_str_length(temp);
        for (unsigned int i = 0; i < zeros_to_add; i++) {
            to[i] = '0';
        }
        for (unsigned int i = 0; i < get_str_length(temp); i++) {
            to[zeros_to_add + i] = temp[i];
        }
        to[desired_length] = '\0';
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
    unsigned int i = 0;
    for (unsigned int ii = start; ii < end+1; ii++) {
        to[i++] = string[ii];
    }
    to[i] = '\0';
}

void join_str(char *to, char *a, char *b) {
    unsigned int i = 0;
    for (unsigned int ii = 0; ii < get_str_length(a); ii++) {
        to[i++] = a[ii];
    }
    for (unsigned int ii = 0; ii < get_str_length(b); ii++) {
        to[i++] = b[ii];
    }
    to[i] = '\0';
}
void str_add_c(char *to, char *string, char c) {
    unsigned int i = 0;
    for (unsigned int ii = 0; ii < get_str_length(string); ii++) {
        to[i++] = string[ii];
    }
    to[i++] = c;
    to[i] = '\0';
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

int str_find(char *string, char *sub, unsigned int start) {
    unsigned int sub_i = 0;
    for(unsigned int i = start; i < get_str_length(string); i++) {
        if (string[i] == sub[sub_i]) {
            sub_i++;
        } else {
            sub_i = 0;
        }
        if (sub_i == get_str_length(sub)) {
            return i-(sub_i-1);
        }
    }
    return -1;
}

int str_find_c(char *string, char c, unsigned int start) {
    for(unsigned int i = start; i < get_str_length(string); i++) {
        if (string[i] == c) {
            return i;
        }
    }
    return -1;
}

void str_replace(char *to, char *original, char *from, char *rep) {
    unsigned int orig_len = get_str_length(original);
    unsigned int from_len = get_str_length(from);
    unsigned int rep_len = get_str_length(rep);
    unsigned int count = str_count(original, from);
    unsigned int out_len = orig_len + count * (rep_len - from_len);
    char out[out_len+1];
    
    unsigned int start_index = 0, i = 0;
    while (start_index < orig_len) {
        int found_at = str_find(original, from, start_index);
        if (found_at == -1) {
            while (start_index < orig_len) {
                out[i++] = original[start_index++];
            }
            break;
        }
        while (start_index < found_at) {
            out[i++] = original[start_index++];
        }
        for (unsigned int ii = 0; ii < rep_len; ii++) {
            out[i++] = rep[ii];
        }
        start_index += from_len;
    }
    out[i] = '\0';
    set_str(to, out);
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

#endif