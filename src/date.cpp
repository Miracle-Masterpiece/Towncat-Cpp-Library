#include <cpp/lang/utils/date.hpp>
#include <cstring>
#include <utility>
#include <cpp/lang/exceptions.hpp>
#include <cerrno>

namespace tc
{

    date::date(int day, int month, int year, int second, int minute, int hour) {
        std::memset(&_localTime, 0, sizeof(_localTime));
         day     = (day     == 0) ? 1 : day;
         month   = (month   == 0) ? 1 : month;
         year    = (year    == 0) ? 1 : year;
        _localTime.tm_year    = year - 1900;
        _localTime.tm_mon     = month - 1;
        _localTime.tm_mday    = day;
        _localTime.tm_sec     = second;
        _localTime.tm_min     = minute;
        _localTime.tm_hour    = hour;
        //_localTime.tm_isdst = -1;

        _time = mktime(&_localTime);
    }

    date::date(time_t date){
        set_time(date);
    }

    date::date(const date& date) : _time(date._time), _localTime(date._localTime) {
        
    }
        
    date::date(date&& date) : _time(std::move(date._time)), _localTime(std::move(date._localTime)) {

    }

    date& date::operator= (const date& date) {
        if (&date != this){
            _time       = date._time;
            _localTime  = date._localTime;
        }
        return *this;
    }
    
    date& date::operator= (date&& date) {
        if (&date != this){
            _time       = std::move(date._time);
            _localTime  = std::move(date._localTime);
        }
        return *this;
    }
    
    date::~date() {
        
    }

    tc::string date::to_string(tca::allocator* alloc) const {
        char buf[128];    
        strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &_localTime);
        return tc::string(buf, alloc);
    }

    /*static*/ date date::now() {
        time_t current;
        std::time(&current);
        return date(current);
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

    void date::set_time(time_t date) {
        _time            = date;
        struct tm* time  = localtime(&_time);
        if (time == nullptr)
            throw_except<runtime_exception>(std::strerror(errno));
        _localTime = *time;
    }

    time_t date::get_time() const {
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