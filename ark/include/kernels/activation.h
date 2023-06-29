// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#ifndef ARK_KERNELS_ACTIVATION_H_
#define ARK_KERNELS_ACTIVATION_H_

#include "ewise.h"

namespace ark {
template <typename InDims, typename OutDims> struct Gelu
{
    template <int NelemPerThread>
    static DEVICE void compute(__half *out, __half *in, int idx_n, int idx_c,
                               int idx_h, int idx_w)
    {
        out += idx_n * OutDims::C * OutDims::H * OutDims::W +
               idx_c * OutDims::H * OutDims::W + idx_h * OutDims::W + idx_w;

        in += idx_n * OutDims::C * OutDims::H * OutDims::W +
              idx_c * OutDims::H * OutDims::W + idx_h * OutDims::W + idx_w;
        __half2 input = *(__half2 *)in;
        __half2 half_pi =
            __float2half2_rn(0.7978845608f); // sqrt(2 / pi) = 0.7978845608
        __half2 coeff = __float2half2_rn(0.044715f);
        __half2 one = __float2half2_rn(1.0f);

        __half2 x_cubed = __hmul2(input, __hmul2(input, input));
        __half2 tanh_input = __hadd2(__hmul2(input, half_pi),
                                     __hmul2(x_cubed, __hmul2(coeff, half_pi)));

        // Convert __half2 to float2
        float2 input_float2 = __half22float2(tanh_input);

        // Compute tanh for each float in the float2 variable
        float2 output_float2 =
            make_float2(tanhf(input_float2.x), tanhf(input_float2.y));

        // Convert float2 back to __half2
        __half2 tanh_output = __float22half2_rn(output_float2);

        __half2 gelu = __hmul2(__hmul2(input, __hadd2(one, tanh_output)),
                               __float2half2_rn(0.5f));
        *(__half2 *)out = gelu;
    }
};

template <typename InDims, typename OutDims, typename OutShape,
          typename UnitOutShape, int ThreadsNum, int SmemBytes>
DEVICE void gelu(ark::half *out, ark::half *in, int tx, int ty, int tz)
{
    constexpr int NelemPerThread = 2;
    Ewise1<InDims, OutDims, OutShape, UnitOutShape, ThreadsNum, SmemBytes,
           Gelu<InDims, OutDims>, __half, NelemPerThread>::run((__half *)out,
                                                               (__half *)in,
                                                               tz / OutShape::C,
                                                               tz % OutShape::C,
                                                               ty, tx);
}

} // namespace ark

#endif // ARK_KERNELS_ACTIVATION_H_