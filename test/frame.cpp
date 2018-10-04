#include "gtest/gtest.h"

#include "renoster/frame.h"
#include "renoster/rng.h"
#include "renoster/sampling.h"

using namespace renoster;

TEST(FrameTest, CreatesBasis)
{
    RNG rng;
    for (int i = 0; i < 10000; ++i) {
        Point2f u(rng.UniformFloat(), rng.UniformFloat());
        Normal3f n = UniformSampleSphere(u);
        Frame frame(n);
        ASSERT_NEAR(frame.s.Length(), 1.f, 1e-4f);
        ASSERT_NEAR(frame.t.Length(), 1.f, 1e-4f);
        ASSERT_NEAR(Dot(frame.s, frame.t), 0.f, 1e-4f);
        ASSERT_NEAR(Dot(frame.s, frame.n), 0.f, 1e-4f);
        ASSERT_NEAR(Dot(frame.t, frame.n), 0.f, 1e-4f);
    }
}

TEST(FrameTest, ToWorldToLocal)
{
    RNG rng;
    for (int i = 0; i < 10; ++i) {
        Point2f u1(rng.UniformFloat(), rng.UniformFloat());
        Normal3f n = UniformSampleSphere(u1);
        Frame frame(n);
        for (int j = 0; j < 100; ++j) {
            Point2f u2(rng.UniformFloat(), rng.UniformFloat());
            Vector3f v = UniformSampleSphere(u2);
            Vector3f w = frame.ToLocal(frame.ToWorld(v));
            ASSERT_NEAR(v.x(), w.x(), 1e-4f);;
            ASSERT_NEAR(v.y(), w.y(), 1e-4f);;
            ASSERT_NEAR(v.z(), w.z(), 1e-4f);;
        }
    }
}

TEST(FrameTest, ToLocalToWorld)
{
    RNG rng;
    for (int i = 0; i < 10; ++i) {
        Point2f u1(rng.UniformFloat(), rng.UniformFloat());
        Normal3f n = UniformSampleSphere(u1);
        Frame frame(n);
        for (int j = 0; j < 100; ++j) {
            Point2f u2(rng.UniformFloat(), rng.UniformFloat());
            Vector3f v = UniformSampleSphere(u2);
            Vector3f w = frame.ToWorld(frame.ToLocal(v));
            ASSERT_NEAR(v.x(), w.x(), 1e-4f);;
            ASSERT_NEAR(v.y(), w.y(), 1e-4f);;
            ASSERT_NEAR(v.z(), w.z(), 1e-4f);;
        }
    }
}
