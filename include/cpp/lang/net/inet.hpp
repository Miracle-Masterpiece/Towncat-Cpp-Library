#ifndef _JSTD_CPP_LANG_NET_INET_H_
#define _JSTD_CPP_LANG_NET_INET_H_


namespace tc {

/**
 * Класс для автоматического управления жизненным циклом сетевой подсистемы.
 * Конструктор класса вызывает функцию init_inet(), а деструктор, соответственно, вызывает close_inet();
 * Попытка создать несколько объектов вызывет illegal_state_exception.
 */
class inet_context {
    friend void init_inet();
    friend void close_inet();
    static bool already_init;
    inet_context(const inet_context&)   = delete;
    inet_context(inet_context&&)        = delete;
    inet_context& operator= (const inet_context&)   = delete;
    inet_context& operator= (inet_context&&)        = delete;
public:
    /**
     * Вызывает функцию init_inet();
     * @see init_inet();
     */
    inet_context();
    
    /**
     * Вызывает функцию close_inet();
     * @see close_inet();
     */
    ~inet_context();
};

    /**
     * Инициализирует сетевую подсистему.
     * 
     * Эта функция выполняет необходимые действия для инициализации сетевой подсистемы
     * на всех поддерживаемых платформах. 
     * На Windows это включает вызов WSAStartup, на Linux и MacOS ничего не требуется.
     * Но на всякий случай лучше вызывать для общего стиля и переносимости.
     * 
     * @throws socket_exception
     *      Если произошла ошибка инициализации.
     * 
     * @throws illegal_state_exception
     *      Если попытка повторной инициализации.
     * 
     * @see inet_context
     */
    void init_inet();
    
    /**
     * Завершает работу сетевой подсистемы.
     * 
     * Эта функция выполняет необходимые действия для корректного завершения работы
     * сетевой подсистемы на всех поддерживаемых платформах. На Windows это включает
     * вызов WSACleanup, на Linux и MacOS ничего не требуется.
     * 
     * @note 
     * Все сокеты должны быть закрыты до вызова этой функции.
     *
     * @throws illegal_state_exception
     *      Если попытка повторного закрытия.
     * 
     * @see inet_context
     */
    void close_inet();
}

#endif//_JSTD_CPP_LANG_NET_INET_H_