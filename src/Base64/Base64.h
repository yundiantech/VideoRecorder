/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef BASE64_H
#define BASE64_H

#include <string>

std::string Base64Encode(unsigned char const* buf, unsigned int len);
std::string Base64Decode(std::string const& s);

#endif // BASE64_H
