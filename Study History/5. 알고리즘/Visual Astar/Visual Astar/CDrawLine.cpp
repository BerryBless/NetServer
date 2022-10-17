#include "CDrawLine.h"

bool CDrawLine::GetLine(int sX, int sY, int eX, int eY, CList<struct stPOS> &path) {
    // 기울기 계산
    int dx = abs(sX - eX);
    int dy = abs(sY - eY);

    int p; // 누적값


    // 좌표를 기울기에 따라 증감할 값
    int ix; // 
    int iy;

    // 실제로 찍힐 좌표
    struct stPOS line;

    if (dy <= dx) {
        // x축이 더김!
        p = 2 * (dy - dx);
        line.Y = sY;

        // 오른쪽으로
        ix = 1;
        if (eX < sX) {
            // 왼쪽으로
            ix = -1;
        }

        // 아래로
        iy = 1;
        if (eY < sY) {
            // 위로
            iy = -1;
        }

        for (line.X = sX; sX <= eX ? line.X <= eX : line.X >= eX; line.X += ix) {
            if (0 >= p) {
                p += 2 * dy;
            } else {
                p += 2 * (dy-dx);
                line.Y += iy;
            }
            path.push_back(line);
        }


    } else {
        // y축이 더김!

        p = 2 * ( dx- dy);
        line.X = sX;

        // 오른쪽으로
        ix = 1;
        if (eX < sX) {
            // 왼쪽으로
            ix = -1;
        }

        // 아래로
        iy = 1;
        if (eY < sY) {
            // 위로
            iy = -1;
        }

        for (line.Y = sY; sY <= eY ? line.Y <= eY : line.Y >= eY; line.Y += iy) {
            if (0 >= p) {
                p += 2 * dx;
            } else {
                p += 2 * (dx - dy);
                line.X += ix;
            }
            path.push_back(line);
        }
    }

    return false;

}
