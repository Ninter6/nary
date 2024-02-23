#pragma once

#include "mathpls.h"
#include "Geometry.hpp"
#include "se_tools.h"

#include <functional>
#include <algorithm>
#include <cmath>
#include <limits>

template<class T, unsigned int N>
struct std::hash<mathpls::vec<T, N>> {
    size_t operator()(const mathpls::vec<T, N>& v) const {
        size_t h = 0;
        for (unsigned int i = 0; i < N; ++i)
            h ^= std::hash<T>()(v[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

template<class T, unsigned int W, unsigned int H>
struct std::hash<mathpls::mat<T, W, H>> {
    size_t operator()(const mathpls::mat<T, W, H>& m) const {
        size_t h = 0;
        for (unsigned int i = 0; i < W; ++i)
            for (unsigned int j = 0; j < H; ++j)
                h ^= std::hash<T>()(m[i][j]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

using Extent2D = mathpls::uivec2;

struct Frustum {
    bool isOverlapping(const pxpls::Sphere& sph) {
        return std::all_of(std::begin(planes), std::end(planes), [&](auto&&i) {
            return !pxpls::IsSphereBelowPlane(sph, i);
        });
    }
    bool isOverlapping(const pxpls::Bounds& bnd) {
        return std::all_of(std::begin(planes), std::end(planes), [&](auto&&i) {
            return !pxpls::IsBoundsBelowPlane(bnd, i);
        });
    }

    pxpls::Plane planes[6];
};
