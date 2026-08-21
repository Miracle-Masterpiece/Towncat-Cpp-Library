#include <cpp/lang/utils/date.hpp>
#include <cstring>
#include <utility>
#include <cpp/lang/exceptions.hpp>
#include <cerrno>

namespace tc
{

    bool get_local_time(const std::time_t* time, std::tm* localtime) {
        JSTD_WIN_CODE
        (
            return localtime_s(localtime, time) == 0;
        )
        JSTD_UNIX_CODE
        (
            return localtime_r(time, localtime) != nullptr;
        )
    }

    date::date(int day, int month, int year, int second, int minute, int hour) {
        std::memset(&_localTime, 0, sizeof(_localTime));
         
        day     = (day     == 0) ? 1       : day;
        month   = (month   == 0) ? 1       : month;
        year    = (year    == 0) ? 1900    : year;
        _localTime.tm_year    = year - 1900;
        _localTime.tm_mon     = month - 1;
        _localTime.tm_mday    = day;
        _localTime.tm_sec     = second;
        _localTime.tm_min     = minute;
        _localTime.tm_hour    = hour;
        _localTime.tm_isdst   = -1;

        _time = mktime(&_localTime);
    }

    date::date(std::time_t date){
        set_time(date);
    }

    tc::string date::to_string(tca::allocator* alloc) const {
        char buf[128];    
        strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &_localTime);
        return tc::string(buf, alloc);
    }

    /*static*/ date date::now() {
        std::time_t current;
        std::time(&current);
        return date(current);
    }

    /*static*/ date date::of_seconds(timepoint sec) {
        return date(static_cast<std::time_t>(sec));
    }

    /*static*/ date date::of_milliseconds(timepoint ms) {
        return of_seconds(ms / 1000);
    }

    bool date::equals(const date& date) const {
        return _time == date._time;
    }

    std::size_t date::hashcode() const {
        return (std::size_t) _time;
    }
    
    int date::compare_to(const date& date) const {
        if (_time < date._time) return -1;
        if (_time > date._time) return  1;
        return 0;
    }

    bool date::operator== (const date& date) const {
        return equals(date);
    }

    bool date::operator!= (const date& date) const {
        return !equals(date);
    }
    
    bool date::operator> (const date& date) const {
        return compare_to(date) > 0;
    }
    
    bool date::operator< (const date& date) const {
        return compare_to(date) < 0;
    }

    bool date::operator>= (const date& date) const {
        return compare_to(date) >= 0;
    }
    
    bool date::operator<= (const date& date) const {
        return compare_to(date) <= 0;
    }

    void date::set_time(std::time_t date) {
        _time            = date;
        if (!get_local_time(&_time, &_localTime))
            throw_except<runtime_exception>(std::strerror(errno));
    }

    std::time_t date::get_time() const {
        return _time;
    }

    int date::get_day() const {
        return _localTime.tm_mday;
    }

    int date::get_month() const {
        return _localTime.tm_mon + 1;
    }

    int date::get_year() const {
        return _localTime.tm_year + 1900;
    }
    
    int date::get_hour() const {
        return _localTime.tm_hour;
    }
    
    int date::get_minute() const {
        return _localTime.tm_min;
    }
    
    int date::get_second() const {
        return _localTime.tm_sec;
    }

    int date::get_week() const {
        return _localTime.tm_wday;
    }

}// namespace jstd