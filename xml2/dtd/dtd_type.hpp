#ifndef  __DTD_TYPE_HPP_1436621815__
#define  __DTD_TYPE_HPP_1436621815__

namespace sss{
    namespace xml2 {
        namespace dtd {
            class Element {
            private:
                std::string name;
                std::string type;
                std::string value_default;
            };

            class Entity {
            private:
                std::string name;
            };

            class ATTList {
            private:
                std::string name;
            };
        }
    }
}


#endif  /* __DTD_TYPE_HPP_1436621815__ */
