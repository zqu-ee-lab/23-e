#ifndef QUADRANGLE_H
#define QUADRANGLE_H
#include <arm_math.h>

struct dot_t
{
    int16_t x;
    int16_t y;
};

#define DOT_NUM 4

struct quadrangle_t
{
    struct dot_t dots[4];
    struct dot_t dots_on_lines[4][DOT_NUM];

    void (*Sort)(struct quadrangle_t *this);
    void (*GetDotsOnLines)(struct quadrangle_t *this);
    void (*unInit)(struct quadrangle_t *this);
    void (*Equal_Scaling)(struct quadrangle_t *this, float32_t scale);
};

struct quadrangle_t *Quadrangle_Init();

struct quadrangle_t *Quadrangle_Init_With_Dots(struct dot_t *dots);

#endif // QUADRANGLE_H