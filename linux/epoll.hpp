#ifndef __EPOLL_HPP_1472962918__
#define __EPOLL_HPP_1472962918__

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

#include <algorithm>
#include <map>
#include <stdexcept>
#include <vector>

#include <sss/errcode.hpp>
#include <sss/error.hpp>
#include <sss/util/PostionThrow.hpp>

namespace sss {
#ifdef linux
#undef linux
#endif
namespace linux {

// template <typename Processor>
class epoll {
    int _epfd;
    uint32_t _fd_cnt;
    // Processor* _pp;
    std::vector<struct epoll_event> _event_list;
    int _ms_timeout;

public:
    enum filter_t {
        MASK_IN = EPOLLIN,
        MASK_OUT = EPOLLOUT,
        MASK_RDHUP = EPOLLRDHUP,
        MASK_PRI = EPOLLPRI,
        MASK_ERR = EPOLLERR,
        MASK_HUP = EPOLLHUP,
        MASK_ET = EPOLLET,
        MASK_ONESHOT = EPOLLONESHOT,
    };

public:
    explicit epoll() : _epfd(0), _fd_cnt(0u), _ms_timeout(-1)
    {
        this->create();
    }
    ~epoll() { this->clear(); }
    epoll& operator=(epoll& ref)
    {
        if (this != &ref) {
            std::swap(this->_epfd, ref._epfd);
        }
        return *this;
    }

public:
    unsigned list_size() const { return this->_fd_cnt; }
    unsigned list_size(unsigned fd_cnt)
    {
        std::swap(fd_cnt, this->_fd_cnt);
        return fd_cnt;
    }
    int timeout() const { return this->_ms_timeout; }
    int timeout(int timeout)
    {
        std::swap(timeout, this->_ms_timeout);
        this->_ms_timeout = std::max(-1, this->_ms_timeout);
        return timeout;
    }

private:
    epoll(const epoll&);
    epoll& operator=(const epoll&);

protected:
    void create() { this->_epfd = ::epoll_create(1); }
    void clear()
    {
        if (this->_epfd) {
            ::close(this->_epfd);
            this->_epfd = 0;
        }
    }

    //   EBADF  epfd or fd is not a valid file descriptor.
    //
    //   EEXIST op was EPOLL_CTL_ADD, and  the  supplied  file  descriptor  fd
    //          is already registered with this epoll instance.
    //
    //   EINVAL epfd  is  not an epoll file descriptor, or fd is the same as
    //          epfd, or the requested operation op is not supported by this
    //          interface.
    //
    //   ENOENT op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, and fd  is  not
    //          registered with this epoll instance.
    //
    //   ENOMEM There  was  insufficient memory to handle the requested op
    //          control operation.
    //
    //   ENOSPC The  limit  imposed  by  /proc/sys/fs/epoll/max_user_watches was
    //          encountered  while  trying  to register (EPOLL_CTL_ADD) a new
    //          file descriptor  on  an  epoll  instance.   See  epoll(7)  for
    //          further details.
    //
    //   EPERM  The target file fd does not support epoll.
    //
public:
    static std::string event2string(uint32_t filter)
    {
        std::string msg;
        struct epoll_mask_name_t : public std::map<int, const char*> {
            epoll_mask_name_t()
            {
#define REGIST_EPOLLMASK_NAME(n) this->operator[](n) = #n
                REGIST_EPOLLMASK_NAME(EPOLLIN);
                REGIST_EPOLLMASK_NAME(EPOLLPRI);
                REGIST_EPOLLMASK_NAME(EPOLLOUT);
                REGIST_EPOLLMASK_NAME(EPOLLRDNORM);
                REGIST_EPOLLMASK_NAME(EPOLLRDBAND);
                REGIST_EPOLLMASK_NAME(EPOLLWRNORM);
                REGIST_EPOLLMASK_NAME(EPOLLWRBAND);
                REGIST_EPOLLMASK_NAME(EPOLLMSG);
                REGIST_EPOLLMASK_NAME(EPOLLERR);
                REGIST_EPOLLMASK_NAME(EPOLLHUP);
                REGIST_EPOLLMASK_NAME(EPOLLRDHUP);
                REGIST_EPOLLMASK_NAME(EPOLLWAKEUP);
                REGIST_EPOLLMASK_NAME(EPOLLONESHOT);
                REGIST_EPOLLMASK_NAME(EPOLLET);
#undef REGIST_EPOLLMASK_NAME
            }
        };
        static epoll_mask_name_t nm_tb;
        for (const auto item : nm_tb) {
            if (filter & item.first) {
                if (!msg.empty()) {
                    msg.append("|");
                }
                msg.append(item.second);
            }
        }
        return msg;
    }
    void call_epoll_ctl(int op, int fd, uint32_t filter, sss::errcode* pec)
    {
        static const char* op_name[] = {
            "EPOLL_CTL_ADD",  // #define EPOLL_CTL_ADD 1
            "EPOLL_CTL_DEL",  // #define EPOLL_CTL_DEL 2
            "EPOLL_CTL_MOD",  // #define EPOLL_CTL_MOD 3
        };
        if (this->_epfd && fd && op >= EPOLL_CTL_ADD && op <= EPOLL_CTL_MOD) {
            struct epoll_event ev;
            ev.data.fd = fd;
            ev.events = filter;
            // printf("epoll_ctl(%d, %s, %d, %s)\n", this->_epfd,
            //        op_name[op - EPOLL_CTL_ADD], fd,
            //        event2string(filter).c_str());
            if (::epoll_ctl(this->_epfd, op, fd, &ev) == -1) {
                std::ostringstream oss;
                oss << "epoll_ctl(" << op_name[op - EPOLL_CTL_ADD]
                    << ") error: " << std::strerror(errno);
                sss::errcode ec(errno, oss.str());
                if (pec) {
                    *pec = ec;
                }
                else {
                    ec.throwMe<std::runtime_error>();
                }
            }
        }
    }

public:
    static bool ensureNonBlocking(int fd)
    {
        int fl = ::fcntl(fd, F_GETFL, 0);
        bool need_mod = !(fl & O_NONBLOCK);
        if (need_mod) {
            ::fcntl(fd, F_SETFL, fl | O_NONBLOCK);
        }
        return need_mod;
    }

    int getEpollFd() const { return _epfd; }
    uint32_t setListSize(uint32_t len)
    {
        std::swap(len, this->_fd_cnt);
        return len;
    }
    uint32_t getListSize() const { return this->_fd_cnt; }
    // NOTE possible errno:
    // EBADF,EEXIST,EINVAL,ENOMEM,ENOSPC,EPERM
    void addFd(int fd,
               uint32_t filter = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET,
               sss::errcode* pec = 0)
    {
        if (this->_epfd && filter & EPOLLET) {
            this->ensureNonBlocking(fd);
        }
        this->call_epoll_ctl(EPOLL_CTL_ADD, fd, filter, pec);
        this->_fd_cnt++;
    }
    // NOTE possible errno:
    // EBADF,EINVAL,ENOENT,ENOMEM,EPERM
    void delFd(int fd, sss::errcode* pec)
    {
        this->call_epoll_ctl(EPOLL_CTL_DEL, fd, 0, pec);
        this->_fd_cnt--;
    }

    // NOTE possible errno:
    // EBADF,EINVAL,ENOENT,ENOMEM,EPERM
    void modFd(int fd, int filter, sss::errcode* pec = 0)
    {
        if (this->_epfd && filter & EPOLLET) {
            this->ensureNonBlocking(fd);
        }
        this->call_epoll_ctl(EPOLL_CTL_MOD, fd, filter, pec);
    }

    /**
     * @brief hasRegister test if fd had been add to epfd interest list
     *
     * @param[in] fd
     *
     * @return bool
     */
    bool hasRegister(int fd)
    {
        bool has_fd = false;
        if (this->_epfd) {
            sss::errcode ec;
            this->call_epoll_ctl(EPOLL_CTL_ADD, fd, 0, &ec);
            if (ec == EEXIST) {
                this->call_epoll_ctl(EPOLL_CTL_DEL, fd, 0, &ec);
                if (ec != 0) {
                    ec.throwMe<std::runtime_error>();
                }
                has_fd = true;
            }
            else if (ec != 0) {
                ec.throwMe<std::runtime_error>();
            }
        }
        return has_fd;
    }

public:
    /**
     * @brief wait call epoll_wait() one-time;
     *
     * @param[out] list
     * @param[out] pec
     *
     * @return event-count number
     */
    int wait(std::vector<struct epoll_event>& list, sss::errcode* pec)
    {
        return wait(list, this->_fd_cnt, pec);
    }

    /**
     * @brief wait 用户提供，等待列表的长度信息；
     *
     * @param list
     * @param event_size
     * @param pec
     *
     * @return
     */
    // EBADF  epfd is not a valid file descriptor.
    //
    // EFAULT The  memory area pointed to by events is not accessible with write
    //        permissions.
    //
    // EINTR  The call was interrupted by a signal handler before either any  of
    //        the  requested  events  occurred  or the timeout expired; see sig‐
    //        nal(7).
    //
    // EINVAL epfd is not an epoll file descriptor, or maxevents is less than or
    //        equal to zero.
    int wait(std::vector<struct epoll_event>& list, int event_size,
             sss::errcode* pec)
    {
        if (this->_epfd && this->_fd_cnt) {
            list.resize(std::max(event_size, 1));
            int ep_wait_cnt =
                ::epoll_wait(this->_epfd, list.data(), this->_fd_cnt,
                             std::max(-1, this->_ms_timeout));
            // printf("%d = epoll_pwait(%d, %p, %d, %d)\n", ep_wait_cnt,
            //        this->_epfd, list.data(), this->_fd_cnt,
            //        std::max(-1, this->_ms_timeout));

            // printf("strerror(%d) = %s\n", errno, strerror(errno));
            // for (int i = 0; i < ep_wait_cnt; ++i) {
            //     printf("%d %s\n", list[i].data.fd,
            //            event2string(list[i].events).c_str());
            // }
            if (ep_wait_cnt >= 0) {
                list.resize(ep_wait_cnt);
            }
            sss::errcode ec;
            if (ep_wait_cnt == -1) {
                std::ostringstream oss;
                oss << "epoll_wait error: " << strerror(errno);
                ec.set(errno, oss.str());
            }
            if (pec) {
                *pec = ec;
            }
            else {
                ec.throwMe<std::runtime_error>();
            }
            return ep_wait_cnt;
        }
        return 0;
    }

    /**
     * @brief loop 循环处理wait()到的事件。
     *
     * NOTE 问题在于，"信息"管理方，和使用方，有些纠结；
     * 在 EPOLL_CTL_ADD 的时候，注册了 interest_fd_list
     * ，并提供了触发的方式mask1。
     * 但是，在 epoll_wait() 这里，得到的信息，却是mask2；有 mask2 <= mask1
     * (这个 "<=" 表示掩码的包含)，即，等到的event，不一定与传入的相等。
     *
     * 并且，如果某次wait中，某fd，可能同时满足多种event！
     *
     * 即，用户的处理函数，应当知道如下信息：
     *  1. int      fd
     *  2. uint32_t filter
     *
     *  再由内部来仔细处理。
     *  即，对于本 sss::epoll 的使用者，他除了要调用 addFd()
     * 等函数外，自己还得记录这些信息。
     * 如果处理函数(对象)，有不同，它自己还得进行区分——比如STDOUT，和STDERR的区别。
     */
    void loop()
    {
        sss::errcode ec;
        while (this->_fd_cnt) {
            int current_waiting = this->wait(this->_event_list, &ec);
            // TODO finish me!
        }
    }

public:
    static inline bool is_event_any_match(struct epoll_event& ev,
                                          sss::linux::epoll::filter_t f)
    {
        return ev.events & f;
    }
    static inline bool is_event_full_match(struct epoll_event& ev,
                                           sss::linux::epoll::filter_t f)
    {
        return ev.events == f;
    }
    static bool is_event_eq_fd(struct epoll_event& ev, int fd)
    {
        return ev.data.fd == fd;
    }
};
epoll::filter_t operator|(epoll::filter_t lhs, epoll::filter_t rhs)
{
    return static_cast<epoll::filter_t>(unsigned(lhs) | unsigned(rhs));
}
}  // namespace linux
}  // namespace sss

#endif /* __EPOLL_HPP_1472962918__ */
