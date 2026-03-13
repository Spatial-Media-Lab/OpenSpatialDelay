#ifndef _structures_h
#define _structures_h

// for FILE

#include <set>
#include <stdlib.h>
#include <vector>

/* copyright 2017 Dr David Sinclair
 david@newtonapples.net
 all rights reserved.


 version of 22-mar-2017.


 this code is released under GPL3,
 a copy of the license can be found at
 http://www.gnu.org/licenses/gpl-3.0.html

 you can purchase a un-restricted license from
 http://newtonapples.net

 where algorithm details are explained.

 If you do choose to purchase a license you might also like
 Newton Apple Chocolates!,
 the cleverest chocolates on the Internet.



 */

struct Tri
{
    int id, keep;
    int a, b, c;
    int ab, bc, ac; // adjacent edges index to neighbouring triangle.
    float er, ec, ez; // visible normal to triangular facet.

    Tri() {}
    Tri (int x, int y, int q) :
        id (0), keep (1), a (x), b (y), c (q), ab (-1), bc (-1), ac (-1), er (0), ec (0), ez (0)
    {
    }
    Tri (const Tri& p) :
        id (p.id),
        keep (p.keep),
        a (p.a),
        b (p.b),
        c (p.c),
        ab (p.ab),
        bc (p.bc),
        ac (p.ac),
        er (p.er),
        ec (p.ec),
        ez (p.ez)
    {
    }

    Tri& operator= (const Tri& p)
    {
        id = p.id;
        keep = p.keep;
        a = p.a;
        b = p.b;
        c = p.c;

        ab = p.ab;
        bc = p.bc;
        ac = p.ac;

        er = p.er;
        ec = p.ec;
        ez = p.ez;

        return *this;
    }
};

struct R3
{
    int id, lspNum, realLspNum = -1;
    float x, y, z;
    float azimuth, elevation, radius;
    bool isImaginary;
    float gain;
    int channel = -1;

    R3() {}
    R3 (float xc, float yc, float zc) : id (-1), x (xc), y (yc), z (zc) {}
    R3 (const R3& p) :
        id (p.id),
        lspNum (p.lspNum),
        realLspNum (p.realLspNum),
        x (p.x),
        y (p.y),
        z (p.z),
        azimuth (p.azimuth),
        elevation (p.elevation),
        radius (p.radius),
        isImaginary (p.isImaginary),
        gain (p.gain),
        channel (p.channel)
    {
    }

    R3& operator= (const R3& p)
    {
        lspNum = p.lspNum;
        realLspNum = p.realLspNum;
        isImaginary = p.isImaginary;
        channel = p.channel;
        gain = p.gain;
        azimuth = p.azimuth;
        elevation = p.elevation;
        radius = p.radius;
        id = p.id;
        x = p.x;
        y = p.y;
        z = p.z;
        return *this;
    }
};

// sort into descending order (for use in corner response ranking).
inline bool operator<(const R3& a, const R3& b)
{
    if (a.z == b.z)
    {
        if (a.x == b.x)
        {
            return a.y < b.y;
        }
        return a.x < b.x;
    }
    return a.z < b.z;
}

struct Snork
{
    int id;
    int a, b;
    Snork() : id (-1), a (0), b (0) {}
    Snork (int i, int r, int x) : id (i), a (r), b (x) {}
    Snork (const Snork& p) : id (p.id), a (p.a), b (p.b) {}

    Snork& operator= (const Snork& p)
    {
        id = p.id;
        a = p.a;
        b = p.b;

        return *this;
    }
};

// sort into descending order (for use in corner response ranking).
inline bool operator<(const Snork& a, const Snork& b)
{
    if (a.a == b.a)
    {
        return a.b < b.b;
    }
    return a.a < b.a;
}

// from NewtonApple_hull3D.cpp

int read_R3 (std::vector<R3>& pts, char* fname);
void write_R3 (std::vector<R3>& pts, char* fname);
void write_Tris (std::vector<Tri>& ts, char* fname);

int de_duplicateR3 (std::vector<R3>& pts, std::vector<int>& outx, std::vector<R3>& pts2);

int NewtonApple_Delaunay (std::vector<R3>& pts, std::vector<Tri>& hulk);
int NewtonApple_hull_3D (std::vector<R3>& pts, std::vector<Tri>& hull);

int init_hull3D (std::vector<R3>& pts, std::vector<Tri>& hull);
void add_coplanar (std::vector<R3>& pts, std::vector<Tri>& hull, int id);
int cross_test (std::vector<R3>& pts, int A, int B, int C, int X, float& er, float& ec, float& ez);
int init_hull3D_compact (std::vector<R3>& pts, std::vector<Tri>& hull);

#endif
