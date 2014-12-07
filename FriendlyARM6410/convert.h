#ifndef CONVERT_H
#define CONVERT_H
#include <qglobal.h>
#include <qsize.h>

QT_BEGIN_NAMESPACE

void qt_convert_NV21_to_ARGB32(const uchar *yuv, quint32 *rgb, int width, int height);

QT_END_NAMESPACE
#endif // CONVERT_H
