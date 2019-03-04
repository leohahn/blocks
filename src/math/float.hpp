#ifndef BLOCKS_MATH_FLOAT_HPP
#define BLOCKS_MATH_FLOAT_HPP

namespace Math
{

inline bool
IsAlmostEqual(float x, float y, float epsilon = std::numeric_limits<float>::epsilon())
{
    // http://realtimecollisiondetection.net/blog/?p=89
    bool almost = abs(x - y) <= epsilon * fmax(fmax(1.0f, abs(x)), abs(y));
    return almost;
}

}

#endif // BLOCKS_MATH_FLOAT_HPP