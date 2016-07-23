#ifndef  __BINIO_HPP_1317660388__
#define  __BINIO_HPP_1317660388__

#include <iosfwd>
#include <cstdio>

namespace sss{

// ȷ�Ϻ���len���ֽڵ����ݣ���pData��ָ���������ͬ
// ��ͬ�Ļ����ƶ��ļ�ָ��
// ��ͬ�Ļ����ļ�ָ���λ�����׳�����Ϊmsg���쳣
void read_require(std::FILE * fp, const void * pData, size_t len, const std::string& msg);
// ȷ���Ѿ������ļ�ĩβ
// ȷ���Ļ����ƶ��ļ�ָ��
// ���ǵĻ����ļ�ָ���λ�����׳�����Ϊmsg���쳣
void require_fseek_at_end(std::FILE * fp, const std::string& msg);

// ��ȡ���е��ַ���ֱ������delimiter�ֽ�
// NOTE -- 2011-10-04 -- Sarrow:�˰汾�У���Ȼ��ȡ��ĩβ���ַ������Ƿ��صĴ����棬��û�а�������
std::string read_stop_at(std::FILE * fp, char delimiter);
// ��ȡ���е��ַ���ֱ������times�Σ���pat_len���ȵ�pattern�飻
// NOTE ���е��ֽڣ�����Ž�����ֵ����
std::string read_stop_at_times(std::FILE * fp, const void * pattern, int pat_len, int times);

long long file2string_c(const std::string& fname, std::string& buffer);

long long file2string_cpp1(const std::string& fname, std::string& buffer);

long long file2string_cpp2(const std::string& fname, std::string& buffer);

}// namespace sss

#endif  /* __BINIO_HPP_1317660388__ */

// std::ifstream
//      ::gcount()
//              ����read()������ȡ���ֽ���
//      ::seekg(...)
//              �ļ�ָ��Ų
//      ::close()
//              �ر�������
//      ::peek()
//              ��ǰ�α�ָ���ֽڣ��α겻��
//
// �򿪷�ʽ��
// ios::in	Ϊ����(��)�����ļ�
// ios::out	Ϊ���(д)�����ļ�
// ios::ate	��ʼλ�ã��ļ�β
// ios::app	��������������ļ�ĩβ
// ios::trunc	����ļ��Ѵ�������ɾ�����ļ�
// ios::binary	�����Ʒ�ʽ
//
// ofstream, ifstream �� fstream��ĳ�Ա����open��Ĭ�Ϸ�ʽ��
// ��	        ������Ĭ�Ϸ�ʽ
// ofstream	ios::out | ios::trunc
// ifstream	ios::in
// fstream	ios::in | ios::out
//
//
// �������������ĸ���ϸ˵��������
//
//! http://blog.chinaunix.net/uid-25749806-id-345845.html
//
// ���� windows,linux�£�ifstream���ı����ļ������ڻ��з��Ĵ���ͬ��ǿ�ҽ���
// ȫ���ö����Ʒ�ʽ���ļ����ر��ǻ����е��ڴ�ģ�ͣ����Բο�EmEditor�Ĵ���ʽ
// ��
