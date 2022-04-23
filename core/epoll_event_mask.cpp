#include "epoll_event_mask.h"
#include "core/string_utils.h"
#include <ostream>
#include <sys/epoll.h>

std::ostream& operator<<(std::ostream& os, const EpollEventMask& evt)
{
    return os << evt.to_string();
}

std::string EpollEventMask::to_string() const
{
    if(!mask)
        return "0";
    std::string ret;
    uint32_t mask_res = mask;
    #define ADD_MASK(MM)\
        do {\
            if(mask_res & MM)\
            {\
                if(ret.length())\
                    ret += "|";\
                ret += #MM;\
                mask_res &= ~MM;\
            }\
        } while(0)
    ADD_MASK(EPOLLERR);
    ADD_MASK(EPOLLPRI);
    ADD_MASK(EPOLLHUP);
    ADD_MASK(EPOLLRDHUP);
    ADD_MASK(EPOLLIN);
    ADD_MASK(EPOLLOUT);
    ADD_MASK(EPOLLRDNORM);
    ADD_MASK(EPOLLRDBAND);
    ADD_MASK(EPOLLWRNORM);
    ADD_MASK(EPOLLWRBAND);
    ADD_MASK(EPOLLMSG);
    ADD_MASK(EPOLLWAKEUP);
    ADD_MASK(EPOLLONESHOT);
    ADD_MASK(EPOLLET);
    if(mask_res)
        ret += format_string("|%x", mask_res);
    return ret;
}
