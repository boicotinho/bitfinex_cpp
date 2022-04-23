#pragma once
#include "core/x_assert.h"
#include "core/gcc_utils.h"
#include <string>
#include <iosfwd>
#include <stdint.h>

// The type of events, epoll will be watching for from a file descriptor.
// Usually only ready_for_read/EPOLLIN will be interesting,
// sometimes also ready_for_write/EPOLLOUT.
// Error events don't need to be include because epoll_wait will always report them.
// one_shot and edge_triggered should be specified by overriding:
//      IEpollBaseEvent::IsOneShot()
//      IEpollBaseEvent::IsEdgeTriggered()
union EpollEventMask
{
    using storage_t = uint32_t; // TODO: use in replacement of pollfd

    constexpr EpollEventMask(const EpollEventMask&) = default;
    constexpr EpollEventMask(uint32_t m=0) : mask(m) {}
    constexpr EpollEventMask( bool a_interested_in_read
                            , bool a_interested_in_write
                            , bool a_edge_triggered    = true
                            , bool a_one_shot          = false
                            )
            : ready_for_read    (a_interested_in_read)
            , priority          (0)
            , ready_for_write   (a_interested_in_write)
            , error             (0)
            , hung_up           (0)
            , reserved_20       (0)
            , rd_norm           (0)
            , rd_band           (0)
            , wr_norm           (0)
            , wr_band           (0)
            , msg               (0)
            , reserved_800      (0)
            , rd_hang_up        (0)
            , reserved_4000     (0)
            , wakeup            (0)
            , one_shot          (a_one_shot)
            , edge_triggered    (a_edge_triggered)
        {}

    bool operator == (const EpollEventMask& rhs) const
    {
        return (mask == rhs.mask);
    };

    bool is_error() const {return error || hung_up;}
    std::string to_string() const;

    uint32_t mask;

    uint16_t pollfd_mask() const {return mask & 0xFFFF;} // for poll rather than epoll

    //error: ISO C++ prohibits anonymous structs [-Werror=pedantic]
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    struct {
        uint32_t ready_for_read  : 1; // EPOLLIN = 0x001,
        uint32_t priority        : 1; // EPOLLPRI = 0x002,
        uint32_t ready_for_write : 1; // EPOLLOUT = 0x004,
        uint32_t error           : 1; // EPOLLERR = 0x008,
        uint32_t hung_up         : 1; // EPOLLHUP = 0x010,
        uint32_t reserved_20     : 1;
        uint32_t rd_norm         : 1; // EPOLLRDNORM = 0x040,
        uint32_t rd_band         : 1; // EPOLLRDBAND = 0x080,
        uint32_t wr_norm         : 1; // EPOLLWRNORM = 0x100,
        uint32_t wr_band         : 1; // EPOLLWRBAND = 0x200,
        uint32_t msg             : 1; // EPOLLMSG    = 0x400,
        uint32_t reserved_800    : 3;
        uint32_t rd_hang_up      : 1; // EPOLLRDHUP  = 0x2000,
        uint32_t reserved_4000   : 14;
        uint32_t wakeup          : 1; // EPOLLWAKEUP = 1u << 29,
        uint32_t one_shot        : 1; // EPOLLONESHOT = 1u << 30,
        uint32_t edge_triggered  : 1; // EPOLLET = 1u << 31
    };
    #pragma GCC diagnostic pop

    STATIC_ASSERT_CLASS_SIZE(4);
} __attribute__ ((__packed__)); // for struct epoll_event __EPOLL_PACKED

std::ostream& operator<<(std::ostream&, const EpollEventMask&);
