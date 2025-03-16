#ifndef TIME_UTILS_H
#define TIME_UTILS_H

struct Time {int sec,min,hour,day,month,year,unix_time; }; typedef struct Time Time;

unsigned int rtc_to_unix(Time t) {
    if (t.year < 1970) return 0;
    // Days since epoch (1970-01-01)
    int days = (t.year - 1970) * 365 + ((t.year - 1969) / 4);
    int month_days[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
    days += month_days[t.month - 1] + (t.day - 1);
    if (t.month > 2 && (t.year % 4 == 0) && (t.year % 100 != 0 || t.year % 400 == 0)) {
        days++;
    }
    return days * 86400 + t.hour * 3600 + t.min * 60 + t.sec;
}

Time unix_to_rtc(int u) {
    Time out;
    out.unix_time = u;
    int days = u / 86400;
    int rem = u % 86400;
    out.hour = rem / 3600;
    rem %= 3600;
    out.min = rem / 60;
    out.sec = rem % 60;
    out.year = 1970;
    while (days >= 365) {
        int leap = (out.year % 4 == 0) && (out.year % 100 != 0 || out.year % 400 == 0);
        if (days < 365 + leap) break;
        days -= 365 + leap;
        (out.year)++;
    }
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((out.year % 4 == 0) && (out.year % 100 != 0 || out.year % 400 == 0)) {
        month_days[1] = 29;
    }
    out.month = 0;
    while (days >= month_days[out.month]) {
        days -= month_days[out.month];
        (out.month)++;
    }
    out.day = days + 1;
    out.month += 1;
    return out;
}

Time time_from_seconds(int seconds) {
    Time t;
    t.unix_time = seconds;
    t.sec = seconds % 60;
    t.min = (seconds / 60) % 60;
    t.hour = (seconds / 3600) % 24;
    t.day = (seconds / 86400) % 30;
    t.month = (seconds / (86400 * 30)) % 12;
    t.year = seconds / (86400 * 365);
    return t;
}

Time get_cmos_time() {
    Time out = {bcd_to_bin(read_cmos(0x00)),bcd_to_bin(read_cmos(0x02)),bcd_to_bin(read_cmos(0x04)),bcd_to_bin(read_cmos(0x07)),bcd_to_bin(read_cmos(0x08)),bcd_to_bin(read_cmos(0x09))+2000};
    out.unix_time = rtc_to_unix(out);
    return out;
}

Time offset_time(Time time, int offset) {
    Time out = unix_to_rtc(time.unix_time + offset);
    return out;
}

#endif