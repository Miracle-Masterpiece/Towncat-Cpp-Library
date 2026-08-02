#ifndef D71091DB_9D04_4B01_A67B_DE42FAC4A290
#define D71091DB_9D04_4B01_A67B_DE42FAC4A290

#include <cerrno>

namespace tc
{

namespace errcode
{

/** The address family (e.g., AF_INET, AF_UNIX) is not supported by the protocol */
#ifdef EAFNOSUPPORT
    const int address_family_not_supported = EAFNOSUPPORT;
#endif

/** The specified address is already in use by another socket */
#ifdef EADDRINUSE
    const int address_in_use = EADDRINUSE;
#endif

/** The requested address is not available on this machine */
#ifdef EADDRNOTAVAIL
    const int address_not_available = EADDRNOTAVAIL;
#endif

// ----------------------------------------------------------------------------
//                      Connection errors
// ----------------------------------------------------------------------------

/** The socket is already connected to a remote endpoint */
#ifdef EISCONN
    const int already_connected = EISCONN;
#endif

/** The connection was forcibly closed by the remote host */
#ifdef ECONNABORTED
    const int connection_aborted = ECONNABORTED;
#endif

/** No process is listening on the remote port, or the connection was rejected */
#ifdef ECONNREFUSED
    const int connection_refused = ECONNREFUSED;
#endif

/** The connection was reset by the remote peer (TCP RST received) */
#ifdef ECONNRESET
    const int connection_reset = ECONNRESET;
#endif

/** The socket is not connected, or the operation requires a connection */
#ifdef ENOTCONN
    const int not_connected = ENOTCONN;
#endif

// ----------------------------------------------------------------------------
//                      Argument and parameter errors
// ----------------------------------------------------------------------------

/** The argument list passed to exec() exceeds system limits (ARG_MAX) */
#ifdef E2BIG
    const int argument_list_too_long = E2BIG;
#endif

/** A mathematical argument is outside the domain of the function (e.g., sqrt(-1)) */
#ifdef EDOM
    const int argument_out_of_domain = EDOM;
#endif

/** The pointer argument points outside the process's accessible address space */
#ifdef EFAULT
    const int bad_address = EFAULT;
#endif

/** An invalid argument was passed to a system call or library function */
#ifdef EINVAL
    const int invalid_argument = EINVAL;
#endif

/** The result of a mathematical operation is too large to be represented */
#ifdef ERANGE
    const int result_out_of_range = ERANGE;
#endif

// ----------------------------------------------------------------------------
//                          File I/O errors
// ----------------------------------------------------------------------------

/** The file descriptor is invalid, closed, or does not refer to an open file */
#ifdef EBADF
    const int bad_file_descriptor = EBADF;
#endif

/** A malformed or invalid message was received (e.g., in STREAMS or sockets) */
#ifdef EBADMSG
    const int bad_message = EBADMSG;
#endif

/** Writing to a pipe or socket with no readers, causing SIGPIPE by default */
#ifdef EPIPE
    const int broken_pipe = EPIPE;
#endif

/** A hardware or filesystem I/O error occurred (e.g., disk read failure) */
#ifdef EIO
    const int io_error = EIO;
#endif

/** No device exists at the specified address, or the device is not present */
#ifdef ENXIO
    const int no_such_device_or_address = ENXIO;
#endif

/** The requested device does not exist in the system */
#ifdef ENODEV
    const int no_such_device = ENODEV;
#endif

/** The specified file or directory does not exist */
#ifdef ENOENT
    const int no_such_file_or_directory = ENOENT;
#endif

/** The path refers to a file, but a directory was expected */
#ifdef ENOTDIR
    const int not_a_directory = ENOTDIR;
#endif

/** The path refers to a directory, but a regular file was expected */
#ifdef EISDIR
    const int is_a_directory = EISDIR;
#endif

/** The operation requires elevated privileges that the process does not have */
#ifdef EPERM
    const int operation_not_permitted = EPERM;
#endif

/** The process does not have the required permissions for the operation */
#ifdef EACCES
    const int permission_denied = EACCES;
#endif

// ----------------------------------------------------------------------------
//                          Filesystem errors
// ----------------------------------------------------------------------------

/** Attempted to rename or link across different filesystem mounts */
#ifdef EXDEV
    const int cross_device_link = EXDEV;
#endif

/** Attempted to remove a directory that still contains files or subdirectories */
#ifdef ENOTEMPTY
    const int directory_not_empty = ENOTEMPTY;
#endif

/** Attempted to create a file that already exists (without O_EXCL flag) */
#ifdef EEXIST
    const int file_exists = EEXIST;
#endif

/** The file size exceeds the maximum allowed by the filesystem or system */
#ifdef EFBIG
    const int file_too_large = EFBIG;
#endif

/** The pathname exceeds the filesystem's maximum filename length limit */
#ifdef ENAMETOOLONG
    const int filename_too_long = ENAMETOOLONG;
#endif

/** The filesystem has no remaining space for new data or metadata */
#ifdef ENOSPC
    const int no_space_on_device = ENOSPC;
#endif

/** Attempted to write to a filesystem mounted with read-only permissions */
#ifdef EROFS
    const int read_only_file_system = EROFS;
#endif

/** Attempted to modify a text file that is currently being executed */
#ifdef ETXTBSY
    const int text_file_busy = ETXTBSY;
#endif

// ----------------------------------------------------------------------------
//                      Process and thread errors
// ----------------------------------------------------------------------------

/** No child processes exist for the parent to wait for (wait()/waitpid()) */
#ifdef ECHILD
    const int no_child_process = ECHILD;
#endif

/** No process exists with the specified process ID (PID) */
#ifdef ESRCH
    const int no_such_process = ESRCH;
#endif

/** A deadlock condition was detected (e.g., with file locking) */
#ifdef EDEADLK
    const int resource_deadlock_would_occur = EDEADLK;
#endif

/** The owner of a robust mutex or other resource has terminated */
#ifdef EOWNERDEAD
    const int owner_dead = EOWNERDEAD;
#endif

/** The state of a robust mutex is not recoverable after owner death */
#ifdef ENOTRECOVERABLE
    const int state_not_recoverable = ENOTRECOVERABLE;
#endif

// ----------------------------------------------------------------------------
//                      Network communication errors
// ----------------------------------------------------------------------------

/** A non-blocking connect() operation is already in progress */
#ifdef EALREADY
    const int connection_already_in_progress = EALREADY;
#endif

/** A destination address must be specified for the operation (e.g., sendto()) */
#ifdef EDESTADDRREQ
    const int destination_address_required = EDESTADDRREQ;
#endif

/** The remote host is not reachable (no route, powered off, or network down) */
#ifdef EHOSTUNREACH
    const int host_unreachable = EHOSTUNREACH;
#endif

/** The local network interface is down or not configured */
#ifdef ENETDOWN
    const int network_down = ENETDOWN;
#endif

/** The network connection was reset by the network stack */
#ifdef ENETRESET
    const int network_reset = ENETRESET;
#endif

/** The network is unreachable (no route to the network) */
#ifdef ENETUNREACH
    const int network_unreachable = ENETUNREACH;
#endif

/** The specified protocol option is not supported by the socket type */
#ifdef ENOPROTOOPT
    const int no_protocol_option = ENOPROTOOPT;
#endif

/** The feature or operation is not supported by the implementation */
#ifdef ENOTSUP
    const int not_supported = ENOTSUP;
#endif

/** The operation is not supported by this socket type or protocol */
#ifdef EOPNOTSUPP
    const int operation_not_supported = EOPNOTSUPP;
#endif

/** The requested protocol is not supported by the system */
#ifdef EPROTONOSUPPORT
    const int protocol_not_supported = EPROTONOSUPPORT;
#endif

/** The socket was created with the wrong protocol type for this operation */
#ifdef EPROTOTYPE
    const int wrong_protocol_type = EPROTOTYPE;
#endif

// ----------------------------------------------------------------------------
//                      Socket and message errors
// ----------------------------------------------------------------------------

/** The file descriptor does not refer to a valid socket */
#ifdef ENOTSOCK
    const int not_a_socket = ENOTSOCK;
#endif

/** The message is too large to be sent in one datagram or exceeds buffer limits */
#ifdef EMSGSIZE
    const int message_size = EMSGSIZE;
#endif

/** No message of the desired type is available on the message queue */
#ifdef ENOMSG
    const int no_message = ENOMSG;
#endif

// ----------------------------------------------------------------------------
//                      Resource and memory errors
// ----------------------------------------------------------------------------

/** The system is out of memory (cannot allocate more virtual memory) */
#ifdef ENOMEM
    const int not_enough_memory = ENOMEM;
#endif

/** The system-wide limit on open files has been reached */
#ifdef ENFILE
    const int too_many_files_open_in_system = ENFILE;
#endif

/** The per-process limit on open file descriptors has been reached */
#ifdef EMFILE
    const int too_many_files_open = EMFILE;
#endif

/** Too many hard links have been created for this file */
#ifdef EMLINK
    const int too_many_links = EMLINK;
#endif

/** Too many symbolic links were encountered while resolving a path */
#ifdef ELOOP
    const int too_many_symbolic_link_levels = ELOOP;
#endif

/** The value is too large for the data type that would store it */
#ifdef EOVERFLOW
    const int value_too_large = EOVERFLOW;
#endif

// ----------------------------------------------------------------------------
//                  Locking and synchronization errors
// ----------------------------------------------------------------------------

/** The system has no available record locks or the lock limit is exceeded */
#ifdef ENOLCK
    const int no_lock_available = ENOLCK;
#endif

/** The link to the file has been severed (e.g., NFS server rebooted) */
#ifdef ENOLINK
    const int no_link = ENOLINK;
#endif

/** The identifier (semaphore, message queue, shared memory) was removed */
#ifdef EIDRM
    const int identifier_removed = EIDRM;
#endif

// ----------------------------------------------------------------------------
//                      Interrupt and timeout errors
// ----------------------------------------------------------------------------

/** The system call was interrupted by a signal before completing */
#ifdef EINTR
    const int interrupted = EINTR;
#endif

/** The resource is temporarily unavailable; retry the operation later */
#ifdef EAGAIN
    const int resource_unavailable_try_again = EAGAIN;
#endif

/** The operation would block the calling thread (non-blocking mode) */
#ifdef EWOULDBLOCK
    const int operation_would_block = EWOULDBLOCK;
#endif

/** The connection or operation has exceeded the timeout limit */
#ifdef ETIMEDOUT
    const int timed_out = ETIMEDOUT;
#endif

/** The operation was explicitly canceled (e.g., with pthread_cancel) */
#ifdef ECANCELED
    const int operation_canceled = ECANCELED;
#endif

// ----------------------------------------------------------------------------
//              Stream errors (obsolete, kept for compatibility)
// ----------------------------------------------------------------------------

/** No message available on the stream (obsolete, replaced by EAGAIN) */
#ifdef ENODATA
    const int no_message_available = ENODATA;
#endif

/** Insufficient stream resources available (obsolete) */
#ifdef ENOSR
    const int no_stream_resources = ENOSR;
#endif

/** The file descriptor does not refer to a STREAMS device (obsolete) */
#ifdef ENOSTR
    const int not_a_stream = ENOSTR;
#endif

/** The stream operation timed out (obsolete) */
#ifdef ETIME
    const int stream_timeout = ETIME;
#endif

// ----------------------------------------------------------------------------
//                      Format and protocol errors
// ----------------------------------------------------------------------------

/** The executable file has an invalid format (e.g., wrong magic number) */
#ifdef ENOEXEC
    const int executable_format_error = ENOEXEC;
#endif

/** The required system call or function is not implemented */
#ifdef ENOSYS
    const int function_not_supported = ENOSYS;
#endif

/** The input contains an illegal multibyte character sequence */
#ifdef EILSEQ
    const int illegal_byte_sequence = EILSEQ;
#endif

/** The file descriptor does not refer to a terminal, but the operation requires one */
#ifdef ENOTTY
    const int inappropriate_io_control_operation = ENOTTY;
#endif

/** The file pointer is not positioned correctly (e.g., seek on a pipe) */
#ifdef ESPIPE
    const int invalid_seek = ESPIPE;
#endif

/** A protocol error occurred in the communication layer */
#ifdef EPROTO
    const int protocol_error = EPROTO;
#endif

// ----------------------------------------------------------------------------
//                      Device and resource errors
// ----------------------------------------------------------------------------

/** The device or resource is busy and cannot be accessed at this time */
#ifdef EBUSY
    const int device_or_resource_busy = EBUSY;
#endif

/** A non-blocking operation is currently in progress (e.g., connect) */
#ifdef EINPROGRESS
    const int operation_in_progress = EINPROGRESS;
#endif

/** The system has no buffer space available for network operations */
#ifdef ENOBUFS
    const int no_buffer_space = ENOBUFS;
#endif
    
} //namespace errcode
} //namespace tc


#endif /* D71091DB_9D04_4B01_A67B_DE42FAC4A290 */
