/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Benchmark.h"
#include "SkColor.h"
#include "SkColorSpaceXformer.h"
#include "SkColorSpaceXformSteps.h"
#include "SkMakeUnique.h"
#include "SkPM4f.h"
#include "SkRandom.h"

enum class Mode { steps, xformer };

struct ColorSpaceXformBench : public Benchmark {
    ColorSpaceXformBench(Mode mode) : fMode(mode) {}

    const Mode fMode;

    std::unique_ptr<SkColorSpaceXformSteps>  fSteps;
    std::unique_ptr<SkColorSpaceXformer>     fXformer;

    const char* onGetName() override {
        switch (fMode) {
            case Mode::steps  : return "ColorSpaceXformBench_steps";
            case Mode::xformer: return "ColorSpaceXformBench_xformer";
        }
        return "";
    }

    bool isSuitableFor(Backend backend) override { return kNonRendering_Backend == backend; }

    void onDelayedSetup() override {
        sk_sp<SkColorSpace> src = SkColorSpace::MakeSRGB(),
                            dst = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                        SkColorSpace::kDCIP3_D65_Gamut);

        fSteps = skstd::make_unique<SkColorSpaceXformSteps>(src.get(), kOpaque_SkAlphaType,
                                                            dst.get(), kPremul_SkAlphaType);
        fXformer = SkColorSpaceXformer::Make(dst);  // src is implicitly sRGB, what we want anyway
    }

    void onDraw(int n, SkCanvas* canvas) override {
        volatile SkColor junk = 0;
        SkRandom rand;

        for (int i = 0; i < n; i++) {
            SkColor src = rand.nextU(),
                    dst;
            switch (fMode) {
                case Mode::steps: {
                    float rgba[4];
                    swizzle_rb(Sk4f_fromL32(src)).store(rgba);
                    fSteps->apply(rgba);
                    dst = Sk4f_toL32(swizzle_rb(Sk4f::Load(rgba)));
                } break;

                case Mode::xformer: {
                    dst = fXformer->apply(src);
                } break;
            }

            if (false && i == 0) {
                SkDebugf("%x ~~> %x\n", src, dst);
            }

            junk ^= dst;
        }
    }
};

DEF_BENCH(return new ColorSpaceXformBench{Mode::steps  };)
DEF_BENCH(return new ColorSpaceXformBench{Mode::xformer};)
