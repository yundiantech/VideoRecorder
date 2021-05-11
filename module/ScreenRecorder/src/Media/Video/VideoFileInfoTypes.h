/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef VideoFileInfoTypes_H
#define VideoFileInfoTypes_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <functional>

struct VideoFileInfo
{
    std::string filePath;
    int64_t length; //时长
    int width;
    int height;
};

#endif // VideoFileInfoTypes_H
