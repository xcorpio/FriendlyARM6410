#include "convert.h"
#include <qlist.h>

QT_BEGIN_NAMESPACE

void qt_convert_NV21_to_ARGB32(const uchar *yuv, quint32 *rgb, int width, int height)
{
    const int frameSize = width * height;

    int a = 0;
    for (int i = 0, ci = 0; i < height; ++i, ci += 1) {
        for (int j = 0, cj = 0; j < width; ++j, cj += 1) {
            int y = (0xff & ((int) yuv[ci * width + cj]));
            int v = (0xff & ((int) yuv[frameSize + (ci >> 1) * width + (cj & ~1) + 0]));
            int u = (0xff & ((int) yuv[frameSize + (ci >> 1) * width + (cj & ~1) + 1]));
            y = y < 16 ? 16 : y;

            int r = (int) (1.164f * (y - 16) + 1.596f * (v - 128));
            int g = (int) (1.164f * (y - 16) - 0.813f * (v - 128) - 0.391f * (u - 128));
            int b = (int) (1.164f * (y - 16) + 2.018f * (u - 128));

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            rgb[a++] = 0xff000000 | (r << 16) | (g << 8) | b;
        }
    }
}

QT_END_NAMESPACE


