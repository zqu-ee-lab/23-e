#include "quadrangle.h" // User/quadrangle.h
#include "FreeRTOS.h"

void Quadrangle_Sort(struct quadrangle_t *this)
{
    // Sort for the clockwise
    struct dot_t the_leftest_dot = this->dots[0];
    int the_leftest_dot_index = 0;
    for (int i = 1; i < 4; i++)
    {
        if (this->dots[i].x < the_leftest_dot.x)
        {
            the_leftest_dot = this->dots[i];
            the_leftest_dot_index = i;
        }
    }
    // the leftest dot is found
    // the leftest dot is the first dot

    struct dot_t temp_dot;
    temp_dot = this->dots[0];
    this->dots[0] = this->dots[the_leftest_dot_index];
    this->dots[the_leftest_dot_index] = temp_dot;

    // calculate the k
    float32_t k[3];
    for (int i = 1; i < 4; i++)
    {
        k[i - 1] = (this->dots[i].y - this->dots[0].y) / (this->dots[i].x - this->dots[0].x);
    }

    // sort the dots
    for (int i = 1; i < 4; i++)
    {
        for (int j = i + 1; j < 4; j++)
        {
            if (k[i - 1] < k[j - 1])
            {
                temp_dot = this->dots[i];
                this->dots[i] = this->dots[j];
                this->dots[j] = temp_dot;
            }
        }
    }

    // the dots are sorted
    return;
}

void Quadrangle_GetDotsOnLines(struct quadrangle_t *this)
{
    // calculate the dots on the lines between the dots
    for (int i = 0; i < 4; i++)
    {
        const int in_dot_num = DOT_NUM;
        for (int j = 0; j < in_dot_num; j++)
        {
            this->dots_on_lines[i][j].x = (-this->dots[i].x + this->dots[(i + 1) % 4].x) / in_dot_num * j + this->dots[i].x;
            this->dots_on_lines[i][j].y = (-this->dots[i].y + this->dots[(i + 1) % 4].y) / in_dot_num * j + this->dots[i].y;
        }
    }
}

struct quadrangle_t *Quadrangle_Init(struct quadrangle_t **this_address)
{
    (*this_address) = (struct quadrangle_t *)pvPortMalloc(sizeof(struct quadrangle_t));
    if ((*this_address) == NULL)
    {
        while (1)
        {
            ;
        }
    }
    (*this_address)->init = Quadrangle_Init;
    (*this_address)->Sort = Quadrangle_Sort;
    (*this_address)->GetDotsOnLines = Quadrangle_GetDotsOnLines;

    return (*this_address);
}

struct quadrangle_t * Quadrangle_Init_With_Dots(struct dot_t *dots)
{
    struct quadrangle_t * this = (struct quadrangle_t *)pvPortMalloc(sizeof(struct quadrangle_t));
    if (this == NULL)
    {
        while (1)
        {
            ;
        }
    }
    this->init = Quadrangle_Init;
    this->Sort = Quadrangle_Sort;
    this->GetDotsOnLines = Quadrangle_GetDotsOnLines;
    for (int i = 0; i < 4; i++)
    {
        this->dots[i] = dots[i];
    }
    this->Sort(this);
    this->GetDotsOnLines(this);

    return this;
}