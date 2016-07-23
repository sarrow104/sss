#include "css_color_table.hpp"

// Ӣ�Ĵ���              ������ɫ                HEX��ʽ                 RGB��ʽ
// --------------------- ----------------------- ----------------------- -----------------------
// LightPink             ǳ�ۺ�                  #FFB6C1                 255,182,193
// Pink                  �ۺ�                    #FFC0CB                 255,192,203
// Crimson               �ɺ�                    #DC143C                 220,20,60
// LavenderBlush         ����ĵ���ɫ            #FFF0F5                 255,240,245
// PaleVioletRed         �԰׵���������ɫ        #DB7093                 219,112,147
// HotPink               ����ķۺ�              #FF69B4                 255,105,180
// DeepPink              ���ɫ                  #FF1493                 255,20,147
// MediumVioletRed       ���е���������ɫ        #C71585                 199,21,133
// Orchid                ��������ɫ              #DA70D6                 218,112,214
// Thistle               ��                      #D8BFD8                 216,191,216
// plum                  ����                    #DDA0DD                 221,160,221
// Violet                ������                  #EE82EE                 238,130,238
// Magenta               ���                    #FF00FF                 255,0,255
// Fuchsia               ��������(�Ϻ�ɫ)        #FF00FF                 255,0,255
// DarkMagenta           �����ɫ                #8B008B                 139,0,139
// Purple                ��ɫ                    #800080                 128,0,128
// MediumOrchid          ���е�������            #BA55D3                 186,85,211
// DarkVoilet            ��������ɫ              #9400D3                 148,0,211
// DarkOrchid            ��������                #9932CC                 153,50,204
// Indigo                ����                    #4B0082                 75,0,130
// BlueViolet            ������������ɫ          #8A2BE2                 138,43,226
// MediumPurple          ���е���ɫ              #9370DB                 147,112,219
// MediumSlateBlue       ���еİ��Ұ�����ɫ      #7B68EE                 123,104,238
// SlateBlue             ���Ұ�����ɫ            #6A5ACD                 106,90,205
// DarkSlateBlue         ���Ұ�����ɫ            #483D8B                 72,61,139
// Lavender              Ѭ�²ݻ��ĵ���ɫ        #E6E6FA                 230,230,250
// GhostWhite            ����İ�ɫ              #F8F8FF                 248,248,255
// Blue                  ����                    #0000FF                 0,0,255
// MediumBlue            ���е���ɫ              #0000CD                 0,0,205
// MidnightBlue          ��ҹ����ɫ              #191970                 25,25,112
// DarkBlue              ����ɫ                  #00008B                 0,0,139
// Navy                  ������                  #000080                 0,0,128
// RoyalBlue             �ʾ���                  #4169E1                 65,105,225
// CornflowerBlue        ʸ���յ���ɫ            #6495ED                 100,149,237
// LightSteelBlue        ������                  #B0C4DE                 176,196,222
// LightSlateGray        ǳʯ���                #778899                 119,136,153
// SlateGray             ʯ���                  #708090                 112,128,144
// DoderBlue             ������                  #1E90FF                 30,144,255
// AliceBlue             ����˿��                #F0F8FF                 240,248,255
// SteelBlue             ����                    #4682B4                 70,130,180
// LightSkyBlue          ����ɫ                  #87CEFA                 135,206,250
// SkyBlue               ����ɫ                  #87CEEB                 135,206,235
// DeepSkyBlue           ������                  #00BFFF                 0,191,255
// LightBLue             ����                    #ADD8E6                 173,216,230
// PowDerBlue            ��ҩ��                  #B0E0E6                 176,224,230
// CadetBlue             ��У��                  #5F9EA0                 95,158,160
// Azure                 ε��ɫ                  #F0FFFF                 240,255,255
// LightCyan             ����ɫ                  #E1FFFF                 225,255,255
// PaleTurquoise         �԰׵��̱�ʯ            #AFEEEE                 175,238,238
// Cyan                  ��ɫ                    #00FFFF                 0,255,255
// Aqua                  ˮ��ɫ                  #00FFFF                 0,255,255
// DarkTurquoise         ���̱�ʯ                #00CED1                 0,206,209
// DarkSlateGray         ��ʯ���                #2F4F4F                 47,79,79
// DarkCyan              ����ɫ                  #008B8B                 0,139,139
// Teal                  ˮѼɫ                  #008080                 0,128,128
// MediumTurquoise       ���е��̱�ʯ            #48D1CC                 72,209,204
// LightSeaGreen         ǳ������                #20B2AA                 32,178,170
// Turquoise             �̱�ʯ                  #40E0D0                 64,224,208
// Auqamarin             ����\����ɫ             #7FFFAA                 127,255,170
// MediumAquamarine      ���еı���ɫ            #00FA9A                 0,250,154
// MediumSpringGreen     ���еĴ������ɫ        #F5FFFA                 245,255,250
// MintCream             ��������                #00FF7F                 0,255,127
// SpringGreen           �������ɫ              #3CB371                 60,179,113
// SeaGreen              ������                  #2E8B57                 46,139,87
// Honeydew              ����                    #F0FFF0                 240,255,240
// LightGreen            ����ɫ                  #90EE90                 144,238,144
// PaleGreen             �԰׵���ɫ              #98FB98                 152,251,152
// DarkSeaGreen          �����                #8FBC8F                 143,188,143
// LimeGreen             �����                  #32CD32                 50,205,50
// Lime                  ���ɫ                  #00FF00                 0,255,0
// ForestGreen           ɭ����                  #228B22                 34,139,34
// Green                 ����                    #008000                 0,128,0
// DarkGreen             ����ɫ                  #006400                 0,100,0
// Chartreuse            ���ؾ���                #7FFF00                 127,255,0
// LawnGreen             ��ƺ��                  #7CFC00                 124,252,0
// GreenYellow           �̻�ɫ                  #ADFF2F                 173,255,47
// OliveDrab             �������ɫ              #556B2F                 85,107,47
// Beige                 ��ɫ(ǳ��ɫ)            #6B8E23                 107,142,35
// LightGoldenrodYellow  ǳ�������              #FAFAD2                 250,250,210
// Ivory                 ����                    #FFFFF0                 255,255,240
// LightYellow           ǳ��ɫ                  #FFFFE0                 255,255,224
// Yellow                ����                    #FFFF00                 255,255,0
// Olive                 ���                    #808000                 128,128,0
// DarkKhaki             ��䲼                #BDB76B                 189,183,107
// LemonChiffon          ���ʱ�ɴ                #FFFACD                 255,250,205
// PaleGodenrod          ��������                #EEE8AA                 238,232,170
// Khaki                 ���䲼                  #F0E68C                 240,230,140
// Gold                  ��                      #FFD700                 255,215,0
// Cornislk              ����ɫ                  #FFF8DC                 255,248,220
// GoldEnrod             ������                  #DAA520                 218,165,32
// FloralWhite           ���İ�ɫ                #FFFAF0                 255,250,240
// OldLace               ���δ�                  #FDF5E6                 253,245,230
// Wheat                 С��ɫ                  #F5DEB3                 245,222,179
// Moccasin              ¹ƤЬ                  #FFE4B5                 255,228,181
// Orange                ��ɫ                    #FFA500                 255,165,0
// PapayaWhip            ��ľ��                  #FFEFD5                 255,239,213
// BlanchedAlmond        Ư�׵�����              #FFEBCD                 255,235,205
// NavajoWhite           Navajo��                #FFDEAD                 255,222,173
// AntiqueWhite          �Ŵ��İ�ɫ              #FAEBD7                 250,235,215
// Tan                   ɹ��                    #D2B48C                 210,180,140
// BrulyWood             ��ʵ����                #DEB887                 222,184,135
// Bisque                Ũ��                    #FFE4C4                 255,228,196
// DarkOrange            ���ɫ                  #FF8C00                 255,140,0
// Linen                 ���鲼                  #FAF0E6                 250,240,230
// Peru                  ��³                    #CD853F                 205,133,63
// PeachPuff             ��ɫ                    #FFDAB9                 255,218,185
// SandyBrown            ɳ��ɫ                  #F4A460                 244,164,96
// Chocolate             �ɿ���                  #D2691E                 210,105,30
// SaddleBrown           ����ɫ                #8B4513                 139,69,19
// SeaShell              ������                  #FFF5EE                 255,245,238
// Sienna                ������ɫ                #A0522D                 160,82,45
// LightSalmon           ǳ����(����)ɫ          #FFA07A                 255,160,122
// Coral                 ɺ��                    #FF7F50                 255,127,80
// OrangeRed             �Ⱥ�ɫ                  #FF4500                 255,69,0
// DarkSalmon            ������(����)ɫ          #E9967A                 233,150,122
// Tomato                ����                    #FF6347                 255,99,71
// MistyRose             ����õ��                #FFE4E1                 255,228,225
// Salmon                ��������ɫ              #FA8072                 250,128,114
// Snow                  ѩ                      #FFFAFA                 255,250,250
// LightCoral            ��ɺ��ɫ                #F08080                 240,128,128
// RosyBrown             õ����ɫ                #BC8F8F                 188,143,143
// IndianRed             ӡ�Ⱥ�                  #CD5C5C                 205,92,92
// Red                   ����                    #FF0000                 255,0,0
// Brown                 ��ɫ                    #A52A2A                 165,42,42
// FireBrick             �ͻ�ש                  #B22222                 178,34,34
// DarkRed               ���ɫ                  #8B0000                 139,0,0
// Maroon                ��ɫ                    #800000                 128,0,0
// White                 ����                    #FFFFFF                 255,255,255
// WhiteSmoke            ����                    #F5F5F5                 245,245,245
// Gainsboro             Gainsboro               #DCDCDC                 220,220,220
// LightGrey             ǳ��ɫ                  #D3D3D3                 211,211,211
// Silver                ����ɫ                  #C0C0C0                 192,192,192
// DarkGray              ���ɫ                  #A9A9A9                 169,169,169
// Gray                  ��ɫ                    #808080                 128,128,128
// DimGray               �����Ļ�ɫ              #696969                 105,105,105
// Black                 ����                    #000000                 0,0,0
//
// vim:
//DSelect $1 $4
//'<,'>s/\(\S\+\)\s\+\(\d\+\),\(\d\+\),\(\d\+\)/(*this)["\1"] = (((\2 << 8) | \3) << 8 | \4);/ge

namespace sss {
css_color_table::css_color_table()
{
    (*this)["lightpink"] = (((255 << 8) | 182) << 8 | 193);
    (*this)["pink"] = (((255 << 8) | 192) << 8 | 203);
    (*this)["crimson"] = (((220 << 8) | 20) << 8 | 60);
    (*this)["lavenderblush"] = (((255 << 8) | 240) << 8 | 245);
    (*this)["palevioletred"] = (((219 << 8) | 112) << 8 | 147);
    (*this)["hotpink"] = (((255 << 8) | 105) << 8 | 180);
    (*this)["deeppink"] = (((255 << 8) | 20) << 8 | 147);
    (*this)["mediumvioletred"] = (((199 << 8) | 21) << 8 | 133);
    (*this)["orchid"] = (((218 << 8) | 112) << 8 | 214);
    (*this)["thistle"] = (((216 << 8) | 191) << 8 | 216);
    (*this)["plum"] = (((221 << 8) | 160) << 8 | 221);
    (*this)["violet"] = (((238 << 8) | 130) << 8 | 238);
    (*this)["magenta"] = (((255 << 8) | 0) << 8 | 255);
    (*this)["fuchsia"] = (((255 << 8) | 0) << 8 | 255);
    (*this)["darkmagenta"] = (((139 << 8) | 0) << 8 | 139);
    (*this)["purple"] = (((128 << 8) | 0) << 8 | 128);
    (*this)["mediumorchid"] = (((186 << 8) | 85) << 8 | 211);
    (*this)["darkvoilet"] = (((148 << 8) | 0) << 8 | 211);
    (*this)["darkorchid"] = (((153 << 8) | 50) << 8 | 204);
    (*this)["indigo"] = (((75 << 8) | 0) << 8 | 130);
    (*this)["blueviolet"] = (((138 << 8) | 43) << 8 | 226);
    (*this)["mediumpurple"] = (((147 << 8) | 112) << 8 | 219);
    (*this)["mediumslateblue"] = (((123 << 8) | 104) << 8 | 238);
    (*this)["slateblue"] = (((106 << 8) | 90) << 8 | 205);
    (*this)["darkslateblue"] = (((72 << 8) | 61) << 8 | 139);
    (*this)["lavender"] = (((230 << 8) | 230) << 8 | 250);
    (*this)["ghostwhite"] = (((248 << 8) | 248) << 8 | 255);
    (*this)["blue"] = (((0 << 8) | 0) << 8 | 255);
    (*this)["mediumblue"] = (((0 << 8) | 0) << 8 | 205);
    (*this)["midnightblue"] = (((25 << 8) | 25) << 8 | 112);
    (*this)["darkblue"] = (((0 << 8) | 0) << 8 | 139);
    (*this)["navy"] = (((0 << 8) | 0) << 8 | 128);
    (*this)["royalblue"] = (((65 << 8) | 105) << 8 | 225);
    (*this)["cornflowerblue"] = (((100 << 8) | 149) << 8 | 237);
    (*this)["lightsteelblue"] = (((176 << 8) | 196) << 8 | 222);
    (*this)["lightslategray"] = (((119 << 8) | 136) << 8 | 153);
    (*this)["slategray"] = (((112 << 8) | 128) << 8 | 144);
    (*this)["doderblue"] = (((30 << 8) | 144) << 8 | 255);
    (*this)["aliceblue"] = (((240 << 8) | 248) << 8 | 255);
    (*this)["steelblue"] = (((70 << 8) | 130) << 8 | 180);
    (*this)["lightskyblue"] = (((135 << 8) | 206) << 8 | 250);
    (*this)["skyblue"] = (((135 << 8) | 206) << 8 | 235);
    (*this)["deepskyblue"] = (((0 << 8) | 191) << 8 | 255);
    (*this)["lightblue"] = (((173 << 8) | 216) << 8 | 230);
    (*this)["powderblue"] = (((176 << 8) | 224) << 8 | 230);
    (*this)["cadetblue"] = (((95 << 8) | 158) << 8 | 160);
    (*this)["azure"] = (((240 << 8) | 255) << 8 | 255);
    (*this)["lightcyan"] = (((225 << 8) | 255) << 8 | 255);
    (*this)["paleturquoise"] = (((175 << 8) | 238) << 8 | 238);
    (*this)["cyan"] = (((0 << 8) | 255) << 8 | 255);
    (*this)["aqua"] = (((0 << 8) | 255) << 8 | 255);
    (*this)["darkturquoise"] = (((0 << 8) | 206) << 8 | 209);
    (*this)["darkslategray"] = (((47 << 8) | 79) << 8 | 79);
    (*this)["darkcyan"] = (((0 << 8) | 139) << 8 | 139);
    (*this)["teal"] = (((0 << 8) | 128) << 8 | 128);
    (*this)["mediumturquoise"] = (((72 << 8) | 209) << 8 | 204);
    (*this)["lightseagreen"] = (((32 << 8) | 178) << 8 | 170);
    (*this)["turquoise"] = (((64 << 8) | 224) << 8 | 208);
    (*this)["auqamarin"] = (((127 << 8) | 255) << 8 | 170);
    (*this)["mediumaquamarine"] = (((0 << 8) | 250) << 8 | 154);
    (*this)["mediumspringgreen"] = (((245 << 8) | 255) << 8 | 250);
    (*this)["mintcream"] = (((0 << 8) | 255) << 8 | 127);
    (*this)["springgreen"] = (((60 << 8) | 179) << 8 | 113);
    (*this)["seagreen"] = (((46 << 8) | 139) << 8 | 87);
    (*this)["honeydew"] = (((240 << 8) | 255) << 8 | 240);
    (*this)["lightgreen"] = (((144 << 8) | 238) << 8 | 144);
    (*this)["palegreen"] = (((152 << 8) | 251) << 8 | 152);
    (*this)["darkseagreen"] = (((143 << 8) | 188) << 8 | 143);
    (*this)["limegreen"] = (((50 << 8) | 205) << 8 | 50);
    (*this)["lime"] = (((0 << 8) | 255) << 8 | 0);
    (*this)["forestgreen"] = (((34 << 8) | 139) << 8 | 34);
    (*this)["green"] = (((0 << 8) | 128) << 8 | 0);
    (*this)["darkgreen"] = (((0 << 8) | 100) << 8 | 0);
    (*this)["chartreuse"] = (((127 << 8) | 255) << 8 | 0);
    (*this)["lawngreen"] = (((124 << 8) | 252) << 8 | 0);
    (*this)["greenyellow"] = (((173 << 8) | 255) << 8 | 47);
    (*this)["olivedrab"] = (((85 << 8) | 107) << 8 | 47);
    (*this)["beige"] = (((107 << 8) | 142) << 8 | 35);
    (*this)["lightgoldenrodyellow"] = (((250 << 8) | 250) << 8 | 210);
    (*this)["ivory"] = (((255 << 8) | 255) << 8 | 240);
    (*this)["lightyellow"] = (((255 << 8) | 255) << 8 | 224);
    (*this)["yellow"] = (((255 << 8) | 255) << 8 | 0);
    (*this)["olive"] = (((128 << 8) | 128) << 8 | 0);
    (*this)["darkkhaki"] = (((189 << 8) | 183) << 8 | 107);
    (*this)["lemonchiffon"] = (((255 << 8) | 250) << 8 | 205);
    (*this)["palegodenrod"] = (((238 << 8) | 232) << 8 | 170);
    (*this)["khaki"] = (((240 << 8) | 230) << 8 | 140);
    (*this)["gold"] = (((255 << 8) | 215) << 8 | 0);
    (*this)["cornislk"] = (((255 << 8) | 248) << 8 | 220);
    (*this)["goldenrod"] = (((218 << 8) | 165) << 8 | 32);
    (*this)["floralwhite"] = (((255 << 8) | 250) << 8 | 240);
    (*this)["oldlace"] = (((253 << 8) | 245) << 8 | 230);
    (*this)["wheat"] = (((245 << 8) | 222) << 8 | 179);
    (*this)["moccasin"] = (((255 << 8) | 228) << 8 | 181);
    (*this)["orange"] = (((255 << 8) | 165) << 8 | 0);
    (*this)["papayawhip"] = (((255 << 8) | 239) << 8 | 213);
    (*this)["blanchedalmond"] = (((255 << 8) | 235) << 8 | 205);
    (*this)["navajowhite"] = (((255 << 8) | 222) << 8 | 173);
    (*this)["antiquewhite"] = (((250 << 8) | 235) << 8 | 215);
    (*this)["tan"] = (((210 << 8) | 180) << 8 | 140);
    (*this)["brulywood"] = (((222 << 8) | 184) << 8 | 135);
    (*this)["bisque"] = (((255 << 8) | 228) << 8 | 196);
    (*this)["darkorange"] = (((255 << 8) | 140) << 8 | 0);
    (*this)["linen"] = (((250 << 8) | 240) << 8 | 230);
    (*this)["peru"] = (((205 << 8) | 133) << 8 | 63);
    (*this)["peachpuff"] = (((255 << 8) | 218) << 8 | 185);
    (*this)["sandybrown"] = (((244 << 8) | 164) << 8 | 96);
    (*this)["chocolate"] = (((210 << 8) | 105) << 8 | 30);
    (*this)["saddlebrown"] = (((139 << 8) | 69) << 8 | 19);
    (*this)["seashell"] = (((255 << 8) | 245) << 8 | 238);
    (*this)["sienna"] = (((160 << 8) | 82) << 8 | 45);
    (*this)["lightsalmon"] = (((255 << 8) | 160) << 8 | 122);
    (*this)["coral"] = (((255 << 8) | 127) << 8 | 80);
    (*this)["orangered"] = (((255 << 8) | 69) << 8 | 0);
    (*this)["darksalmon"] = (((233 << 8) | 150) << 8 | 122);
    (*this)["tomato"] = (((255 << 8) | 99) << 8 | 71);
    (*this)["mistyrose"] = (((255 << 8) | 228) << 8 | 225);
    (*this)["salmon"] = (((250 << 8) | 128) << 8 | 114);
    (*this)["snow"] = (((255 << 8) | 250) << 8 | 250);
    (*this)["lightcoral"] = (((240 << 8) | 128) << 8 | 128);
    (*this)["rosybrown"] = (((188 << 8) | 143) << 8 | 143);
    (*this)["indianred"] = (((205 << 8) | 92) << 8 | 92);
    (*this)["red"] = (((255 << 8) | 0) << 8 | 0);
    (*this)["brown"] = (((165 << 8) | 42) << 8 | 42);
    (*this)["firebrick"] = (((178 << 8) | 34) << 8 | 34);
    (*this)["darkred"] = (((139 << 8) | 0) << 8 | 0);
    (*this)["maroon"] = (((128 << 8) | 0) << 8 | 0);
    (*this)["white"] = (((255 << 8) | 255) << 8 | 255);
    (*this)["whitesmoke"] = (((245 << 8) | 245) << 8 | 245);
    (*this)["gainsboro"] = (((220 << 8) | 220) << 8 | 220);
    (*this)["lightgrey"] = (((211 << 8) | 211) << 8 | 211);
    (*this)["silver"] = (((192 << 8) | 192) << 8 | 192);
    (*this)["darkgray"] = (((169 << 8) | 169) << 8 | 169);
    (*this)["gray"] = (((128 << 8) | 128) << 8 | 128);
    (*this)["dimgray"] = (((105 << 8) | 105) << 8 | 105);
    (*this)["black"] = (((0 << 8) | 0) << 8 | 0);
}

css_color_table::~css_color_table()
{}

css_color_table& css_color_table::get_singleton()
{
    static css_color_table color2int;
    return color2int;
}

bool css_color_table::is_valid_name(const std::string& name)
{
    css_color_table& table = css_color_table::get_singleton();
    std::map<std::string, int>::iterator it = table.find(name);
    return it != table.end();
}

int css_color_table::convert(const std::string& name)
{
    if (css_color_table::is_valid_name(name))
    {
        return css_color_table::get_singleton()[name];
    }
    return -1;
}

} // end of namespace sss
