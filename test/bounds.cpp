#include "gtest/gtest.h"

#include "renoster/bounds.h"

using namespace renoster;

TEST(BoundsTest, UnionDegenerate)
{
    Bounds3f a(Point3f(-10.f), Point3f(20.f));
    Bounds3f b;
    EXPECT_EQ(a, Union(a, b));
    EXPECT_EQ(a, Union(b, a));
}

TEST(BoundsTest, UnionIdempotent)
{
    Bounds3f b(Point3f(-10.f), Point3f(10.f));
    EXPECT_EQ(b, Union(b, b));
}

TEST(BoundsTest, UnionCommutative)
{
    Bounds3f a(Point3f(-10.f), Point3f(10.f));
    Bounds3f b(Point3f(0.f), Point3f(20.f));
    EXPECT_EQ(Union(a, b), Union(b, a));
}

TEST(BoundsTest, UnionAssociative)
{
    Bounds3f a(Point3f(-10.f), Point3f(10.f));
    Bounds3f b(Point3f(0.f), Point3f(20.f));
    Bounds3f c(Point3f(10.f), Point3f(30.f));
    EXPECT_EQ(Union(a, Union(b, c)), Union(Union(a, b), c));
}
