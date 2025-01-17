// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#ifndef ARK_KERNELS_GEMM_CUTLASS_H_
#define ARK_KERNELS_GEMM_CUTLASS_H_

// clang-format off
#include "cutlass/cutlass.h"
#include "cute/tensor.hpp"

#include "cutlass/numeric_types.h"

#include "cutlass/gemm/device/gemm.h"
#include "cutlass/gemm/device/gemm_universal_adapter.h"
#include "cutlass/gemm/kernel/gemm_universal.hpp"
#include "cutlass/epilogue/collective/collective_builder.hpp"
#include "cutlass/gemm/collective/collective_builder.hpp"
#include "cutlass/epilogue/collective/sm70_epilogue_vectorized.hpp"
#include "cutlass/epilogue/collective/default_epilogue.hpp"
#include "cutlass/epilogue/thread/linear_combination.h"
// clang-format on

#include "common/checker.h"
#include "common/unit_op.h"

namespace ark {

/// Custom ThreadblockSwizzle for ARK.
template <typename UnitOp>
struct GemmThreadblockSwizzle {
    DEVICE GemmThreadblockSwizzle() {}

    DEVICE cutlass::gemm::GemmCoord get_tiled_shape() const {
        return cutlass::gemm::GemmCoord(UnitOp::UnitOpDims::H,
                                        UnitOp::UnitOpDims::W, 1);
    }

    DEVICE int get_log_tile(cutlass::gemm::GemmCoord) const { return 0; }

    DEVICE cutlass::gemm::GemmCoord get_tile_offset(int log_tile) const {
        // log_tile is actually uop_idx here.
        int uh = UnitOp::uop_idx_h(log_tile);
        int uw = UnitOp::uop_idx_w(log_tile);
        return cutlass::gemm::GemmCoord{uh, uw, 0};
    }
};

template <typename UnitOp, typename OperatorClass, typename ArchTag,
          typename ElementA, typename LayoutA, typename ElementB,
          typename LayoutB, typename ElementC, typename LayoutC, typename Shape>
struct GemmConfiguration;

////////////////////////////////////////////////////////////////////////////////
/// SM70 FP16
////////////////////////////////////////////////////////////////////////////////

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm70, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<128, 256, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<128, 256, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>, cutlass::gemm::GemmShape<8, 8, 4>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 2>;
};

////////////////////////////////////////////////////////////////////////////////
/// SM80 FP16
////////////////////////////////////////////////////////////////////////////////

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<128, 256, 64>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<128, 256, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<256, 128, 64>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<256, 128, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<128, 128, 64>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<128, 128, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<256, 64, 64>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<256, 64, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<64, 256, 64>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<64, 256, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<64, 128, 64>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<64, 128, 64>,
        cutlass::gemm::GemmShape<32, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<128, 64, 64>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<128, 64, 64>,
        cutlass::gemm::GemmShape<64, 32, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<64, 64, 64>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<32, 32, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 6>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<128, 256, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<128, 256, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<256, 128, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<256, 128, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<128, 128, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<128, 128, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<256, 64, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<256, 64, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<64, 256, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<64, 256, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<64, 128, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<64, 128, 32>,
        cutlass::gemm::GemmShape<32, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 6>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<128, 64, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<128, 64, 32>,
        cutlass::gemm::GemmShape<64, 32, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 6>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<64, 64, 32>> {
    using ElementOutput = cutlass::half_t;
    using ElementAccumulator = cutlass::half_t;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<32, 32, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 10>;
};

////////////////////////////////////////////////////////////////////////////////
/// SM80 BF16
////////////////////////////////////////////////////////////////////////////////

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<128, 256, 64>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<128, 256, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<256, 128, 64>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<256, 128, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<128, 128, 64>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<128, 128, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<256, 64, 64>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<256, 64, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<64, 256, 64>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<64, 256, 64>,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<64, 128, 64>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<64, 128, 64>,
        cutlass::gemm::GemmShape<32, 64, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<128, 64, 64>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<128, 64, 64>,
        cutlass::gemm::GemmShape<64, 32, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<64, 64, 64>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<64, 64, 64>,
        cutlass::gemm::GemmShape<32, 32, 64>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 6>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<128, 256, 32>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<128, 256, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<256, 128, 32>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<256, 128, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<128, 128, 32>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<128, 128, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<256, 64, 32>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<256, 64, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<64, 256, 32>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<64, 256, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 4>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<64, 128, 32>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<64, 128, 32>,
        cutlass::gemm::GemmShape<32, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 6>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<128, 64, 32>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::bfloat16_t, LayoutA, cutlass::bfloat16_t, LayoutB,
        ElementOutput, LayoutC, ElementAccumulator,
        cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<128, 64, 32>,
        cutlass::gemm::GemmShape<64, 32, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 6>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, cutlass::bfloat16_t, LayoutA,
                         cutlass::bfloat16_t, LayoutB, cutlass::bfloat16_t,
                         LayoutC, cutlass::gemm::GemmShape<64, 64, 32>> {
    using ElementOutput = cutlass::bfloat16_t;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        cutlass::half_t, LayoutA, cutlass::half_t, LayoutB, ElementOutput,
        LayoutC, ElementAccumulator, cutlass::arch::OpClassTensorOp,
        cutlass::arch::Sm80, cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<32, 32, 32>,
        cutlass::gemm::GemmShape<16, 8, 16>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 10>;
};

////////////////////////////////////////////////////////////////////////////////
/// SM80 FP32
////////////////////////////////////////////////////////////////////////////////

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<
    UnitOp, cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80, float, LayoutA,
    float, LayoutB, float, LayoutC, cutlass::gemm::GemmShape<128, 256, 32>> {
    using ElementOutput = float;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        float, LayoutA, float, LayoutB, ElementOutput, LayoutC,
        ElementAccumulator, cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<128, 256, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 8>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<
    UnitOp, cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80, float, LayoutA,
    float, LayoutB, float, LayoutC, cutlass::gemm::GemmShape<128, 128, 32>> {
    using ElementOutput = float;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        float, LayoutA, float, LayoutB, ElementOutput, LayoutC,
        ElementAccumulator, cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<128, 128, 32>,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<16, 8, 8>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm80, float, LayoutA, float, LayoutB,
                         float, LayoutC, cutlass::gemm::GemmShape<64, 64, 32>> {
    using ElementOutput = float;
    using ElementAccumulator = float;

    using Gemm = cutlass::gemm::device::Gemm<
        float, LayoutA, float, LayoutB, ElementOutput, LayoutC,
        ElementAccumulator, cutlass::arch::OpClassTensorOp, cutlass::arch::Sm80,
        cutlass::gemm::GemmShape<64, 64, 32>,
        cutlass::gemm::GemmShape<32, 32, 32>,
        cutlass::gemm::GemmShape<16, 8, 8>,
        cutlass::epilogue::thread::LinearCombination<
            ElementOutput, 128 / cutlass::sizeof_bits<ElementOutput>::value,
            ElementAccumulator, ElementAccumulator>,
        ark::GemmThreadblockSwizzle<UnitOp>, 3>;
};

#if 0
template <typename UnitOp>
struct GemmConfiguration<
    UnitOp, cutlass::arch::OpClassTensorOp, cutlass::arch::Sm90,
    cutlass::half_t, cutlass::layout::RowMajor, cutlass::half_t,
    cutlass::layout::RowMajor, cutlass::half_t, cutlass::layout::ColumnMajor,
    cutlass::gemm::GemmShape<64, 128, 64>>
{
    using LayoutA = cutlass::layout::RowMajor;
    using LayoutB = cutlass::layout::RowMajor;
    using LayoutC = cutlass::layout::ColumnMajor;

    using CollectiveOp = typename cutlass::gemm::collective::CollectiveBuilder<
        cutlass::arch::Sm90, cutlass::arch::OpClassTensorOp, cutlass::half_t,
        LayoutA, 8, cutlass::half_t, LayoutB, 8, cutlass::half_t,
        cute::Shape<cute::_64, cute::_128, cute::_64>, cute::Shape<cute::_1, cute::_1, cute::_1>,
        cutlass::gemm::collective::StageCountAuto,
        cutlass::gemm::collective::KernelScheduleAuto>::CollectiveOp;

    using CollectiveEpilogue =
        typename cutlass::epilogue::collective::CollectiveBuilder<
            cutlass::arch::Sm90, cutlass::arch::OpClassTensorOp,
            cute::Shape<cute::_64, cute::_128, cute::_64>, cute::Shape<cute::_1, cute::_1, cute::_1>,
            cutlass::epilogue::collective::EpilogueTileAuto, cutlass::half_t,
            cutlass::half_t, cutlass::half_t, LayoutC, 8, cutlass::half_t,
            LayoutC, 8,
            cutlass::epilogue::collective::EpilogueScheduleAuto>::CollectiveOp;

    using GemmKernel =
        cutlass::gemm::kernel::GemmUniversal<cute::Shape<int, int, int, int>,
                                             CollectiveOp, CollectiveEpilogue>;

    using Gemm = cutlass::gemm::device::GemmUniversalAdapter<GemmKernel>;
};

template <typename UnitOp>
struct GemmConfiguration<
    UnitOp, cutlass::arch::OpClassTensorOp, cutlass::arch::Sm90,
    cutlass::half_t, cutlass::layout::RowMajor, cutlass::half_t,
    cutlass::layout::RowMajor, cutlass::half_t, cutlass::layout::ColumnMajor,
    cutlass::gemm::GemmShape<128, 128, 32>>
{
    using LayoutA = cutlass::layout::RowMajor;
    using LayoutB = cutlass::layout::RowMajor;
    using LayoutC = cutlass::layout::ColumnMajor;

    using CollectiveOp = typename cutlass::gemm::collective::CollectiveBuilder<
        cutlass::arch::Sm90, cutlass::arch::OpClassTensorOp, cutlass::half_t,
        LayoutA, 8, cutlass::half_t, LayoutB, 8, cutlass::half_t,
        cute::Shape<cute::_128, cute::_128, cute::_32>, cute::Shape<cute::_1, cute::_1, cute::_1>,
        cutlass::gemm::collective::StageCountAuto,
        cutlass::gemm::collective::KernelScheduleAuto>::CollectiveOp;

    using CollectiveEpilogue =
        typename cutlass::epilogue::collective::CollectiveBuilder<
            cutlass::arch::Sm90, cutlass::arch::OpClassTensorOp,
            cute::Shape<cute::_128, cute::_128, cute::_32>, cute::Shape<cute::_1, cute::_1, cute::_1>,
            cutlass::epilogue::collective::EpilogueTileAuto, cutlass::half_t,
            cutlass::half_t, cutlass::half_t, LayoutC, 8, cutlass::half_t,
            LayoutC, 8,
            cutlass::epilogue::collective::EpilogueScheduleAuto>::CollectiveOp;

    using GemmKernel =
        cutlass::gemm::kernel::GemmUniversal<cute::Shape<int, int, int, int>,
                                             CollectiveOp, CollectiveEpilogue>;

    using Gemm = cutlass::gemm::device::GemmUniversalAdapter<GemmKernel>;
};

template <typename UnitOp, typename LayoutA, typename LayoutB, typename LayoutC>
struct GemmConfiguration<UnitOp, cutlass::arch::OpClassTensorOp,
                         cutlass::arch::Sm90, cutlass::half_t, LayoutA,
                         cutlass::half_t, LayoutB, cutlass::half_t, LayoutC,
                         cutlass::gemm::GemmShape<64, 64, 64>>
{
    using CollectiveOp = typename cutlass::gemm::collective::CollectiveBuilder<
        cutlass::arch::Sm90, cutlass::arch::OpClassTensorOp, cutlass::half_t,
        LayoutA, 8, cutlass::half_t, LayoutB, 8, cutlass::half_t,
        cute::Shape<cute::_64, cute::_64, cute::_64>, cute::Shape<cute::_1, cute::_1, cute::_1>,
        cutlass::gemm::collective::StageCountAuto,
        cutlass::gemm::collective::KernelScheduleAuto>::CollectiveOp;

    using CollectiveEpilogue =
        typename cutlass::epilogue::collective::CollectiveBuilder<
            cutlass::arch::Sm90, cutlass::arch::OpClassTensorOp,
            cute::Shape<cute::_64, cute::_64, cute::_64>, cute::Shape<cute::_1, cute::_1, cute::_1>,
            cutlass::epilogue::collective::EpilogueTileAuto, cutlass::half_t,
            cutlass::half_t, cutlass::half_t, LayoutC, 8, cutlass::half_t,
            LayoutC, 8,
            cutlass::epilogue::collective::EpilogueScheduleAuto>::CollectiveOp;

    using GemmKernel =
        cutlass::gemm::kernel::GemmUniversal<cute::Shape<int, int, int, int>,
                                             CollectiveOp, CollectiveEpilogue>;

    using Gemm = cutlass::gemm::device::GemmUniversalAdapter<GemmKernel>;
};
#endif

/// CUDA GeMM for arch equal to or earlier than 80.
template <typename DataTypeA, int LeadingDimA, bool IsColumnA,
          typename DataTypeB, int LeadingDimB, bool IsColumnB,
          typename DataTypeC, int LeadingDimC, int ProblemSizeM,
          int ProblemSizeN, int ProblemSizeK, int TileSizeM, int TileSizeN,
          int TileSizeK, typename UnitOp>
DEVICE void gemm_cuda(DataTypeC *C, DataTypeA *A, DataTypeB *B, int uop_idx,
                      int smem_per_warp) {
#if (ARK_TARGET_CUDA_ARCH == 60)
    using ArchTag = cutlass::arch::Sm60;
#elif (ARK_TARGET_CUDA_ARCH == 70)
    using ArchTag = cutlass::arch::Sm70;
#elif (ARK_TARGET_CUDA_ARCH == 80)
    using ArchTag = cutlass::arch::Sm80;
#elif (ARK_TARGET_CUDA_ARCH == 90)
    static_assert(false, "Use gemm_cuda_90 instead.");
#else
    static_assert(false, "Unsupported CUDA arch.");
#endif

    using LayoutA = typename cutlass::platform::conditional<
        IsColumnA, cutlass::layout::ColumnMajor,
        cutlass::layout::RowMajor>::type;
    using LayoutB = typename cutlass::platform::conditional<
        IsColumnB, cutlass::layout::ColumnMajor,
        cutlass::layout::RowMajor>::type;
    using LayoutC = cutlass::layout::RowMajor;

    using GemmKernel = typename ark::GemmConfiguration<
        UnitOp, cutlass::arch::OpClassTensorOp, ArchTag, DataTypeA, LayoutA,
        DataTypeB, LayoutB, DataTypeC, LayoutC,
        cutlass::gemm::GemmShape<TileSizeM, TileSizeN,
                                 TileSizeK>>::Gemm::GemmKernel;

    IsEq<GemmKernel::kThreadCount, UnitOp::NumThreads>();
    IsEq<sizeof(GemmKernel::SharedStorage), UnitOp::SmemBytes>();

    LayoutA layout_a(LeadingDimA);
    LayoutB layout_b(LeadingDimB);
    LayoutC layout_c(LeadingDimC);
    cutlass::TensorRef<DataTypeA, LayoutA> ref_a(A, layout_a);
    cutlass::TensorRef<DataTypeB, LayoutB> ref_b(B, layout_b);
    cutlass::TensorRef<DataTypeC, LayoutC> ref_c(C, layout_c);

    cutlass::gemm::GemmCoord problem_size(ProblemSizeM, ProblemSizeN,
                                          ProblemSizeK);
    cutlass::gemm::GemmCoord threadblock_shape(TileSizeM, TileSizeN, TileSizeK);

    ark::GemmThreadblockSwizzle<UnitOp> swizzle;
    cutlass::gemm::GemmCoord tiled_shape(swizzle.get_tiled_shape());

    typename GemmKernel::Params params(problem_size, tiled_shape, ref_a, ref_b,
                                       ref_c, ref_c);

    // A hack for custom threadblock swizzle. swizzle_log_tile is useless
    // for ARK, instead we need uop_idx to determine the tile offset.
    // Since swizzle_log_tile is the input to get_tile_offset(), we can
    // use it to pass uop_idx.
    params.swizzle_log_tile = uop_idx;

    typename GemmKernel::SharedStorage *ps =
        UnitOp::template shared_memory<GemmKernel::SharedStorage>(
            smem_per_warp);

    GemmKernel gemm_kernel{};
    gemm_kernel(params, *ps);
}

/// Row-major GeMM.
template <typename DataTypeA, int LeadingDimA, bool IsColumnA,
          typename DataTypeB, int LeadingDimB, bool IsColumnB,
          typename DataTypeC, int LeadingDimC, int ProblemSizeM,
          int ProblemSizeN, int ProblemSizeK, int TileSizeM, int TileSizeN,
          int TileSizeK, typename UnitOp>
DEVICE void gemm_cutlass(DataTypeC *C, DataTypeA *A, DataTypeB *B, int uop_idx,
                         int smem_per_warp) {
    using CutDataTypeA = typename cutlass::platform::conditional<
        std::is_same<DataTypeA, fp16>::value, cutlass::half_t,
        typename cutlass::platform::conditional<
            std::is_same<DataTypeA, bf16>::value, cutlass::bfloat16_t,
            DataTypeA>::type>::type;

    using CutDataTypeB = typename cutlass::platform::conditional<
        std::is_same<DataTypeB, fp16>::value, cutlass::half_t,
        typename cutlass::platform::conditional<
            std::is_same<DataTypeB, bf16>::value, cutlass::bfloat16_t,
            DataTypeB>::type>::type;

    using CutDataTypeC = typename cutlass::platform::conditional<
        std::is_same<DataTypeC, fp16>::value, cutlass::half_t,
        typename cutlass::platform::conditional<
            std::is_same<DataTypeC, bf16>::value, cutlass::bfloat16_t,
            DataTypeC>::type>::type;

    CutDataTypeC *pC = reinterpret_cast<CutDataTypeC *>(C);
    CutDataTypeA *pA = reinterpret_cast<CutDataTypeA *>(A);
    CutDataTypeB *pB = reinterpret_cast<CutDataTypeB *>(B);

#if (ARK_TARGET_CUDA_ARCH == 60 || ARK_TARGET_CUDA_ARCH == 70 || \
     ARK_TARGET_CUDA_ARCH == 80)
    gemm_cuda<CutDataTypeA, LeadingDimA, IsColumnA, CutDataTypeB, LeadingDimB,
              IsColumnB, CutDataTypeC, LeadingDimC, ProblemSizeM, ProblemSizeN,
              ProblemSizeK, TileSizeM, TileSizeN, TileSizeK, UnitOp>(
        pC, pA, pB, uop_idx, smem_per_warp);
#elif (ARK_TARGET_CUDA_ARCH == 90)
    gemm_cuda_90<CutDataTypeA, LeadingDimA, IsColumnA, CutDataTypeB,
                 LeadingDimB, IsColumnB, CutDataTypeC, LeadingDimC,
                 ProblemSizeM, ProblemSizeN, ProblemSizeK, TileSizeM, TileSizeN,
                 TileSizeK, UnitOp>(pC, pA, pB, uop_idx, smem_per_warp);
#else
    static_assert(false, "Unsupported CUDA arch.");
#endif
}

}  // namespace ark

#endif  // ARK_KERNELS_GEMM_CUTLASS_H_
