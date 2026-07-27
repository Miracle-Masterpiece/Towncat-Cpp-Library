#include <internal/bsd_socket_class.hpp>
#include <internal/bsd_socket.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <iostream>

namespace jstd 
{
namespace bsd_socket 
{

    typedef std::intptr_t SOCKET_HANDLE_TYPE;

    sock_handle::sock_handle() : handle((SOCKET_HANDLE_TYPE) NULL_SOCKET) {

    }
    
    sock_handle::sock_handle(std::intptr_t handle) : handle(handle) {

    }
    
    sock_handle::sock_handle(sock_handle&& id) {
        std::swap(handle, id.handle);
    }
    
    sock_handle& sock_handle::operator= (sock_handle&& id) {
        if (&id != this)
        {
            std::swap(handle, id.handle);
        }
        return *this;
    }

    sock_handle::~sock_handle() {

    }

    void sock_handle::close() {
        if ((SOCK_TYPE) handle == NULL_SOCKET)
            return;
        try {
            bsd_socket::close((SOCK_TYPE) handle);
            utils::assign_static_cast(handle, NULL_SOCKET);
        } catch (...) {
            utils::assign_static_cast(handle, NULL_SOCKET);
            throw;
        }
    }

/**
 * =====================================================================================================================================================================
 * =====================================================================================================================================================================
 */

    socket_impl::socket_impl() : base_socket<sock_handle>(), _blocking(true) {
        
    }
    
    socket_impl::socket_impl(socket_impl&& sock) : base_socket<sock_handle>(std::move(sock)), _blocking(sock._blocking) {

    }

    socket_impl& socket_impl::operator= (socket_impl&& sock) {
        if (&sock != this)
        {
            base_socket<sock_handle>::operator=(std::move(sock));
            _blocking = sock._blocking;
        }
        return *this;
    }

    void socket_impl::set_socket_option(int opt_id, const socket_option& value) {
        bsd_socket::set_sock_opt((SOCK_TYPE) sock_id.handle, opt_id, value);
    }
    
    socket_option socket_impl::get_socket_option(int opt_id) const {
        socket_option ret;
        bsd_socket::get_sock_opt((SOCK_TYPE) sock_id.handle, opt_id, ret);
        return ret;
    }
    
    void socket_impl::connect(const char* host, unsigned int port) {
        connect(inet_address(host), port);
    }

    void socket_impl::connect(const inet_address& host, unsigned int port) {
        bsd_socket::connect((SOCK_TYPE) sock_id.handle, host, port);
        socket_address addr;
        bsd_socket::get_sock_name((SOCK_TYPE) sock_id.handle, addr);
        set_port(port);
        set_address(host);
        set_localport(addr.get_port());
    }
    
    void socket_impl::bind(const inet_address& address, unsigned int port) {
        bsd_socket::bind((SOCK_TYPE) sock_id.handle, address, port);
    }

    void socket_impl::listen(int backlog) {
        bsd_socket::listen((SOCK_TYPE) sock_id.handle, backlog);
    }

    bool socket_impl::accept(base_socket<sock_handle>* client) {
        JSTD_DEBUG_CODE(
            if (client == nullptr)
                throw_except<null_pointer_exception>("client is nullptr!");
        )
        
        socket_address client_address;
        SOCK_TYPE client_sock_id = bsd_socket::accept((SOCK_TYPE) sock_id.handle, &client_address);
        
        if (client_sock_id != NULL_SOCKET)
        {
            client->set_address(client_address.get_address());
            client->set_localport(port);
            client->set_port(client_address.get_port());
            client->set_descriptor(sock_handle((SOCKET_HANDLE_TYPE) client_sock_id));
            return true;
        }
            
        return false;
    }
    
    void socket_impl::create(inet_family family) {
        sock_handle sockID((SOCKET_HANDLE_TYPE) bsd_socket::open_tcp(family));
        sock_id = std::move(sockID); 
    }
    
    bool socket_impl::is_created() const {
        return (SOCK_TYPE) sock_id.handle != NULL_SOCKET;
    }

    void socket_impl::close() {
        sock_id = sock_handle((SOCKET_HANDLE_TYPE) NULL_SOCKET);
    }
    
    void socket_impl::shutdown_in() {
        bsd_socket::shutdown_in((SOCK_TYPE) sock_id.handle);
    }
    
    void socket_impl::shutdown_out() {
        bsd_socket::shutdown_out((SOCK_TYPE) sock_id.handle);
    }
    
    std::size_t socket_impl::read(char buf[], std::size_t len) {
        return bsd_socket::recv((SOCK_TYPE) sock_id.handle, buf, len, _blocking);
    }
    
    std::size_t socket_impl::write(const char* data, std::size_t len) {
        return bsd_socket::send((SOCK_TYPE) sock_id.handle, data, len, _blocking);
    }

    void socket_impl::set_blocking(bool is_block) {
        bsd_socket::set_blocking((SOCK_TYPE) sock_id.handle, is_block);
        _blocking = is_block;
    }
    
    bool socket_impl::is_blocking() const {
        return _blocking;
    }

    socket_impl::~socket_impl() {
        
    }

}

}