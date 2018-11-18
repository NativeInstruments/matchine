# Benchmarks

## Type Hierarchy

These are some performance comparisons casting of types build with `type_hierarchy` (TH) vs classical RTTI based
`dynamic_cast` using `ni::match`.

`l0`, `l1`, `l2` referes to objects of types on the respective inheritence level, i.e. root, 1st inherited level,
and 2nd inherited level, repsecitvely. Times have been measured on a Macbook Pro 13" i5 2 core (Apple clang 9.0)
and a Windows machine using MSVC 2017.


| cast           | RTTI (clang) | TH (clang) | TH speed-up (clang) | RTTI (msvc) | TH (msvc) | speed-up (msvc) |
|----------------|--------------|------------|---------------------|-------------|-----------|-----------------|
| l0 -> l1 succ. |  13 ns       |  1.2 ns    |  11 x faster        |  24 ns      |  4.8 ns   |   5 x faster    |
| l0 -> l1 fail  |  19 ns       |  1.2 ns    |  16 x faster        |  43 ns      |  4.8 ns   |   9 x faster    |
| l1 -> l2 succ. |  14 ns       |  1.2 ns    |  12 x faster        |  24 ns      |  4.8 ns   |   5 x faster    |
| l1 -> l2 fail  |  19 ns       |  1.2 ns    |  16 x faster        |  52 ns      |  4.8 ns   |  11 x faster    |
| l0 -> l2 succ. |  15 ns       |  1.2 ns    |  12 x faster        |  35 ns      |  4.8 ns   |   7 x faster    |
| l0 -> l2 fail  |  21 ns       |  1.2 ns    |  17 x faster        |  52 ns      |  4.8 ns   |  11 x faster    |
| l2 -> l1 match |  1.2 ns      |  1.2 ns    |   1 x faster        |  4.8 ns     |  4.8 ns   |   1 x faster    |
| l2 -> l1 fail  |  16 ns       |  1.2 ns    |  13 x faster        |  52 ns      |  4.8 ns   |  11 x faster    |

