#ifndef JSTD_CPP_LANG_IO_IOSTREAM_H
#define JSTD_CPP_LANG_IO_IOSTREAM_H



namespace tc
{
    /**
     * Подавляет все исключения при закрытии потока ввода.
     * 
     * @param in
     *      Поток ввода у которого будет вызвана функция close.
     *      И если close завершится с ошибкой, проигнорирует её.
     */
    void close_stream_and_suppress_except(class istream* in);

    /**
     * Подавляет все исключения при закрытии потока вывода.
     * 
     * @param out
     *      Поток вывода у которого будет вызвана функция close.
     *      И если close завершится с ошибкой, проигнорирует её.
     */
    void close_stream_and_suppress_except(class ostream* out);
}

#endif//JSTD_CPP_LANG_IO_IOSTREAM_H