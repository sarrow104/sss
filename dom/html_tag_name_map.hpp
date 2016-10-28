#ifndef  __HTML_TAG_NAME_MAP_HPP_1390914166__
#define  __HTML_TAG_NAME_MAP_HPP_1390914166__

#include <string>
#include <map>

namespace sss { namespace dom {
class html_tag_name_map : private std::map<int, std::string>{
public:
    enum html_tag_id {
        //id_HEAD = 1,

        //id_HTML,        // 2
        //id_TITLE,       // 3
        //id_META,        // 4

        //id_BODY,        // 5

        //id_H1,          // 6
        //id_H2,          // 7
        //id_H3,          // 8
        //id_H4,          // 9
        //id_H5,          // 10
        //id_H6,          // 11

        //id_TABLE,       // 12
        //id_TR,          // 13
        //id_TH,          // 14
        //id_TD,          // 15

        //id_P,           // 16

        //id_HR,          // 17
        //id_BR,          // 18

        //id_STYLE,       // 19
        id_A           = 1,
        id_ABBR,
        id_ACRONYM,
        id_ADDRESS,
        id_AREA,
        id_BASE,
        id_BDO,
        id_BLOCKCODE,
        id_BODY,
        id_BR,
        id_BUTTON,
        id_CAPTION,
        id_CITE,
        id_CODE,
        id_COL,
        id_COLGROUP,
        id_DD,
        id_DEL,
        id_DFN,
        id_DIV,
        id_DL,
        id_DT,
        id_EM,
        id_FIELDSET,
        id_FORM,
        id_FRAME,
        id_FRAMESET,
        id_H1,
        id_H2,
        id_H3,
        id_H4,
        id_H5,
        id_H6,
        id_HEAD,
        id_HR,
        id_HTML,
        id_IFRAME,
        id_IMG,
        id_INPUT,
        id_INS,
        id_KBD,
        id_L,
        id_LABEL,
        id_LEGEND,
        id_LI,
        id_LINK,
        id_MAP,
        id_META,
        id_NOFRAMES,
        id_NOSCRIPT,
        id_OBJECT,
        id_OL,
        id_OPTION,
        id_P,
        id_PARAM,
        id_PRE,
        id_QUOTE,
        id_Q,
        id_SAMP,
        id_SCRIPT,
        id_SECTION,
        id_SELECT,
        id_SEPARATOR,
        id_SPAN,
        id_STRONG,
        id_STYLE,
        id_SUB,
        id_SUP,
        id_TABLE,
        id_TBODY,
        id_TD,
        id_TEXTAREA,
        id_TFOOT,
        id_TH,
        id_THEAD,
        id_TITLE,
        id_TR,
        id_UL,
        id_VAR
    };

private:
    // 另外，各html_tag子类型，除了绑定tag_name和is_single外，还应该绑定打印风格；
    // 比如，div，是block；而h1,...,h6是inline；
    // 反映在打印上，就是div的begin_tag和end_tag单独一行；而h1,...,h6等，一般公用一行；
    //
    // 这样的话，就需要三元组对象；
    //
    // 当然，还是有变通的办法：检测内部elements的数量；如果为"空"，那么就公用一行；
    html_tag_name_map() {
// vim:'<,'>s/^\s*\<\(id_\)\(\w\+\)\>,/(*this)[id_\2] = std::make_pair<std::string, bool>("\U\2", false);/ge
//        (*this)[id_HTML] = "html";
//
//        (*this)[id_HEAD] = "head";
//        (*this)[id_TITLE] = "title";
//        (*this)[id_META] = "meta";
//
//        (*this)[id_BODY] = "body";
//
//        (*this)[id_H1] = "h1";
//        (*this)[id_H2] = "h2";
//        (*this)[id_H3] = "h3";
//        (*this)[id_H4] = "h4";
//        (*this)[id_H5] = "h5";
//        (*this)[id_H6] = "h6";
//
//
//        (*this)[id_TABLE] = "table";
//        (*this)[id_TR] = "tr";
//        (*this)[id_TH] = "th";
//        (*this)[id_TD] = "td";
//
//        (*this)[id_P] = "p";
//
//        (*this)[id_HR] = "hr";
//        (*this)[id_BR] = "br";
//
//        (*this)[id_STYLE] = "style";
        (*this)[id_A] = "a";
        (*this)[id_ABBR] = "abbr";
        (*this)[id_ACRONYM] = "acronym";
        (*this)[id_ADDRESS] = "address";
        (*this)[id_AREA] = "area";
        (*this)[id_BASE] = "base";
        (*this)[id_BDO] = "bdo";
        (*this)[id_BLOCKCODE] = "blockcode";
        (*this)[id_BODY] = "body";
        (*this)[id_BR] = "br";
        (*this)[id_BUTTON] = "button";
        (*this)[id_CAPTION] = "caption";
        (*this)[id_CITE] = "cite";
        (*this)[id_CODE] = "code";
        (*this)[id_COL] = "col";
        (*this)[id_COLGROUP] = "colgroup";
        (*this)[id_DD] = "dd";
        (*this)[id_DEL] = "del";
        (*this)[id_DFN] = "dfn";
        (*this)[id_DIV] = "div";
        (*this)[id_DL] = "dl";
        (*this)[id_DT] = "dt";
        (*this)[id_EM] = "em";
        (*this)[id_FIELDSET] = "fieldset";
        (*this)[id_FORM] = "form";
        (*this)[id_FRAME] = "frame";
        (*this)[id_FRAMESET] = "frameset";
        (*this)[id_H1] = "h1";
        (*this)[id_H2] = "h2";
        (*this)[id_H3] = "h3";
        (*this)[id_H4] = "h4";
        (*this)[id_H5] = "h5";
        (*this)[id_H6] = "h6";
        (*this)[id_HEAD] = "head";
        (*this)[id_HR] = "hr";
        (*this)[id_HTML] = "html";
        (*this)[id_IFRAME] = "iframe";
        (*this)[id_IMG] = "img";
        (*this)[id_INPUT] = "input";
        (*this)[id_INS] = "ins";
        (*this)[id_KBD] = "kbd";
        (*this)[id_L] = "l";
        (*this)[id_LABEL] = "label";
        (*this)[id_LEGEND] = "legend";
        (*this)[id_LI] = "li";
        (*this)[id_LINK] = "link";
        (*this)[id_MAP] = "map";
        (*this)[id_META] = "meta";
        (*this)[id_NOFRAMES] = "noframes";
        (*this)[id_NOSCRIPT] = "noscript";
        (*this)[id_OBJECT] = "object";
        (*this)[id_OL] = "ol";
        (*this)[id_OPTION] = "option";
        (*this)[id_P] = "p";
        (*this)[id_PARAM] = "param";
        (*this)[id_PRE] = "pre";
        (*this)[id_QUOTE] = "quote";
        (*this)[id_Q] = "q";
        (*this)[id_SAMP] = "samp";
        (*this)[id_SCRIPT] = "script";
        (*this)[id_SECTION] = "section";
        (*this)[id_SELECT] = "select";
        (*this)[id_SEPARATOR] = "separator";
        (*this)[id_SPAN] = "span";
        (*this)[id_STRONG] = "strong";
        (*this)[id_STYLE] = "style";
        (*this)[id_SUB] = "sub";
        (*this)[id_SUP] = "sup";
        (*this)[id_TABLE] = "table";
        (*this)[id_TBODY] = "tbody";
        (*this)[id_TD] = "td";
        (*this)[id_TEXTAREA] = "textarea";
        (*this)[id_TFOOT] = "tfoot";
        (*this)[id_TH] = "th";
        (*this)[id_THEAD] = "thead";
        (*this)[id_TITLE] = "title";
        (*this)[id_TR] = "tr";
        (*this)[id_UL] = "ul";
        (*this)[id_VAR] = "var";
    }

    static html_tag_name_map& get_instance() {
        static html_tag_name_map my_map;
        return my_map;
    }

public:
    static const std::string& get_name_by_id(int id) {
        return get_instance()[id];
    }

    //static bool        get_tag_is_single(int id) {
    //    return get_instance()[id].second;
    //}
};

}} // end of namespace sss::dom

#endif  /* __HTML_TAG_NAME_MAP_HPP_1390914166__ */
