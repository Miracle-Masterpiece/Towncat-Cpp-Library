
#include <cpp/lang/logging/logger.hpp>
#include <cpp/lang/utils/date.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstring>

namespace tc
{

    namespace log
    {
        
        logger::logger() : m_out(nullptr) {

        }

        logger::logger(ostream* out) : m_out(out) {
            JSTD_DEBUG_CODE(check_non_null(out));
        }
        
        logger::logger(logger&& l) : m_out(l.m_out) {

        }
        
        logger& logger::operator= (logger&& l) {
            if (&l != this) {
                l       = l.m_out;
                l.m_out = nullptr;
            }
            return *this;
        }
        
        logger::~logger() {

        }

        void logger::message(level lvl, const char* msg) {
            JSTD_DEBUG_CODE(check_non_null(msg));

            const std::size_t RESULT_BUFFER_SIZE = 129;
            char result[RESULT_BUFFER_SIZE];
            int result_size = 0;

            tc::string strdate = date::now().to_string();

            switch (lvl) {
                case level::INFO : {
                    result_size = std::snprintf(result, RESULT_BUFFER_SIZE, "[INFO] (%s) %s\n", strdate.cstr(), msg);
                    break;
                }
                case level::WARN : {
                    result_size = std::snprintf(result, RESULT_BUFFER_SIZE, "[WARNING] (%s) %s\n", strdate.cstr(), msg);
                    break;
                }
                case level::ERROR : {
                    result_size = std::snprintf(result, RESULT_BUFFER_SIZE, "[ERROR] (%s) %s\n", strdate.cstr(), msg);
                    break;
                }
                case level::EXCEPT : {
                    result_size = std::snprintf(result, RESULT_BUFFER_SIZE, "[EXCEPTION] (%s) %s\n", strdate.cstr(), msg);
                    break;
                }
            }

        if (m_out == nullptr)
        {
            system::tsprintf(result);
        }
        else
        {
            try {
                m_out->write(result, (std::size_t) result_size);
            } catch (const io_exception& e) {
                system::tsprintf("%s\n", e.cause());
                e.print_stack_trace();
            }
        }

        }

        void logger::warn(const char* msg) {
            message(level::WARN, msg);
        }
    
        void logger::info(const char* msg) {
            message(level::INFO, msg);
        }
        
        void logger::error(const char* msg) {
            message(level::ERROR, msg);
        }

        void logger::except(const throwable& t) {
            message(level::EXCEPT, t.cause());
        }
    }
}