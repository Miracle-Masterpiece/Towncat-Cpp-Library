#ifndef JSTD_CPP_LANG_LOGGING_LOGGER_H
#define JSTD_CPP_LANG_LOGGING_LOGGER_H

namespace tc
{
    class throwable;
    class ostream;
    namespace log
    {
        enum struct level {
            INFO,   /* информация       */
            WARN,   /* предупреждение   */
            ERROR,  /* ошибка           */
            EXCEPT  /* исключение       */
        };
    
        /**
         * Класс логгера для логирования.
         */
        class logger {
            ostream* m_out;   /* выходной поток в который сохраняются логи */
            
            logger(const logger&)               = delete;
            logger& operator= (const logger&)   = delete;
            
            /**
             * @internal
             * Сохраняет сообщение в выходной поток.
             * Если выходной поток равен null, сообщение будет записано в jstd::system::tsprintf
             * 
             * @param lvl
             *      Уровень сообщения.
             * 
             * @param mgs
             *      Сообщение лога.
             * 
             * @throws null_pointer_exception
             *      Если msg равно nullptr.
             */
            void message(level lvl, const char* msg);

        public:
            /**
             * Создаёт логгер, сообщения которого будут логироваться в консоль.
             */
            logger();
            
            /**
             * Создаёт логгер, сообщения которого будут записаны в выходной поток.
             * 
             * @param out
             *      Поток вывода, в который будут записываться сообщения.
             */
            logger(ostream* out);
            
            /**
             * Конструктор перемещения.
             * 
             * @param other
             *      Объект из которого будут перемещены данные.
             */
            logger(logger&& other);
            
            /**
             * Оператор перемещения.
             * 
             * @param other
             *      Объект из которого будут перемещены данные.
             * 
             * @return
             *      Этот объект.
             */
            logger& operator= (logger&& other);
            
            /**
             * 
             */
            ~logger();
            
            /**
             * Записывает строку с отметкой предупрежления.
             * 
             * @param msg
             *      Строка, которая будет записана в лог в качестве предупреждения.
             *      Строка должна оканчиваться на нуль-терминатор!
             * 
             * @throws null_pointer_exception
             *      Если msg равно nullptr;
             */
            void warn(const char* msg);
            
            /**
             * Записывает строку с отметкой информации.
             * 
             * @param msg
             *      Строка, которая будет записана в лог в качестве информации.
             *      Строка должна оканчиваться на нуль-терминатор!
             * 
             * @throws null_pointer_exception
             *      Если msg равно nullptr;
             */
            void info(const char* msg);
            
            /**
             * Записывает строку с отметкой ошибки.
             * 
             * @param msg
             *      Строка, которая будет записана в лог в качестве ошибки.
             *      Строка должна оканчиваться на нуль-терминатор!
             * 
             * @throws null_pointer_exception
             *      Если msg равно nullptr;
             */
            void error(const char* msg);
            
            /**
             * Записывает строку с отметкой исключения.
             * 
             * @param msg
             *      Строка, которая будет записана в лог в качестве исключения.
             */
            void except(const throwable&);
        };
    }
}

#endif//JSTD_CPP_LANG_LOGGING_LOGGER_H