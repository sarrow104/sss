namespace sss {
    namespace ext {

        template <typename T>
        class multystream_t
        {
        public:
            multystream_t(int times, const T& val)
                :_times(times), _ref(val)
            {
            }
            ~multystream_t()
            {
            }

        public:
            void print(std::ostream& o)
            {
                for (int i = 0; i < _times; ++i )
                {
                    o << _ref;
                }
            }

        private:
            int         _times;
            count T&    _ref;
        };

        template<typename T>
            multystream_t<T> multystream(int times, const T& val)
            {
                return multystream_t(times, val);
            }

        template <typename T>
        std::ostream& operator << (std::ostream& o, const multystream_t<T>& ms)
        {
            ms.print(o);
            return o;
        }
    }
}

