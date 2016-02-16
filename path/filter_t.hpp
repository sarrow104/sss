#ifndef  __FILTER_T_HPP_1449558847__
#define  __FILTER_T_HPP_1449558847__

namespace sss {
    namespace path {

        // TODO 过滤器，应当通过外部传入；
        // 该对象可以接受一个 const file_descriptor& 对象；
        // 然后，过滤器，特别针对正则表达式，有优化！
        class filter_t
        {
        public:
            filter_t()
            {
            }
            virtual ~filter_t()
            {
            }

        public:
            virtual bool is_match(const file_descriptor& fd) = 0;
        };

        // NOTE 用户可以据此，创建多种 filter ；然后实际的工作类，可以从这些filter 进行继承；
        // 貌似要用到虚基类；
        // 工作类::is_match() {
        //   return (name_filter_t::is_match(fd) && crttime_filter_t::is_match(fd) && ...);
        // }
        // 貌似可行，不过看起来很傻；

    }
}



#endif  /* __FILTER_T_HPP_1449558847__ */
