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
		else if (this->dots[i].x == the_leftest_dot.x)
		{
			if (this->dots[i].y < the_leftest_dot.y)
			{
				the_leftest_dot = this->dots[i];
				the_leftest_dot_index = i;
			}
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
		float32_t dx = this->dots[i].x - this->dots[0].x;
		if (dx == 0)
		{
			dx = .0001f;
		}
		k[i - 1] = (this->dots[i].y - this->dots[0].y) / dx;
	}

	// sort the dots
	for (int i = 1; i < 4; i++)
	{
		for (int j = i + 1; j < 4; j++)
		{
			if (k[i - 1] > k[j - 1])
			{
				temp_dot = this->dots[i];
				this->dots[i] = this->dots[j];
				this->dots[j] = temp_dot;
				float32_t temp_k = k[i - 1];
				k[i - 1] = k[j - 1];
				k[j - 1] = temp_k;
			}
		}
	}

	// the second dot is offset outwards, so move it inwards base on the fourth dot
	float32_t dx = this->dots[1].x - this->dots[3].x;
	if (dx == 0)
	{
		dx = .0001f;
	}
	float32_t dy = this->dots[1].y - this->dots[3].y;
	if (dy == 0)
	{
		dy = .0001f;
	}
	float32_t k_1_3 = dy / dx;
	this->dots[1].y = k_1_3 * dx * .98 + this->dots[3].y;
	this->dots[1].x = dy / k_1_3 * .98 + this->dots[3].x;

	// the dots are sorted
	return;
}

// void Quadrangle_GetDotsOnLines(struct quadrangle_t *this)
// {
//     // calculate the dots on the lines between the dots
//     for (int i = 0; i < 4; i++)
//     {
//         const int in_dot_num = DOT_NUM-1;
//         for (int j = 0; j < in_dot_num; j++)
//         {
//             this->dots_on_lines[i][j].x = (-this->dots[i].x + this->dots[(i + 1) % 4].x) / in_dot_num * j + this->dots[i].x;
//             this->dots_on_lines[i][j].y = (-this->dots[i].y + this->dots[(i + 1) % 4].y) / in_dot_num * j + this->dots[i].y;
//         }
//         this->dots_on_lines[i][in_dot_num] = this->dots[(i + 1) % 4];
//     }
// }

struct dot_t Quadrangle_GetSpectialDotOnLines(struct quadrangle_t *this, int line_index, int dot_index)
{
	struct dot_t dot_local;
	if (dot_index == DOT_NUM - 1)
	{
		// dot_local = this->dots[(line_index + 1) % 4];
		return this->dots[(line_index + 1) % 4];
	}
	dot_local.x = (-this->dots[line_index].x + this->dots[(line_index + 1) % 4].x) / (DOT_NUM - 1) * dot_index + this->dots[line_index].x;
	dot_local.y = (-this->dots[line_index].y + this->dots[(line_index + 1) % 4].y) / (DOT_NUM - 1) * dot_index + this->dots[line_index].y;
	if (line_index >= 2)
	{
		double center_x = 0, center_y = 0;
		for (int i = 0; i < 4; i++)
		{
			center_x += this->dots[i].x;
			center_y += this->dots[i].y;
		}
		center_x /= 4;
		center_y /= 4;
		dot_local.x = (dot_local.x - center_x) * .92 + center_x;
		dot_local.y = (dot_local.y - center_y) * .92 + center_y;
	}
	if(line_index == 3|| line_index == 2){
		if(dot_index == 0){
			dot_local.x = dot_local.x * .96 + this->dots[line_index-1].x * .04;
			dot_local.y = dot_local.y * .96 + this->dots[line_index-1].y * .04;
		}
	}
	return dot_local;
}

void Quadrangle_Equal_Scaling(struct quadrangle_t *this, float32_t scale)
{
	// calculate the center dot
	double center_x = 0, center_y = 0;
	for (int i = 0; i < 4; i++)
	{
		center_x += this->dots[i].x;
		center_y += this->dots[i].y;
	}
	center_x /= 4;
	center_y /= 4;

	// scale the dots
	for (int i = 0; i < 4; i++)
	{
		this->dots[i].x = (this->dots[i].x - center_x) * scale + center_x;
		this->dots[i].y = (this->dots[i].y - center_y) * scale + center_y;
	}
}

void Quadrangle_unInit(struct quadrangle_t *this)
{
	vPortFree(this);
}

struct quadrangle_t *Quadrangle_Init()
{
	struct quadrangle_t *this = (struct quadrangle_t *)pvPortMalloc(sizeof(struct quadrangle_t));
	if (this == NULL)
	{
		while (1)
		{
			;
		}
	}
	this->Sort = Quadrangle_Sort;
	// this->GetDotsOnLines = Quadrangle_GetDotsOnLines;
	this->GetSpectialDotOnLines = Quadrangle_GetSpectialDotOnLines;
	this->unInit = Quadrangle_unInit;
	this->Equal_Scaling = Quadrangle_Equal_Scaling;

	return this;
}

struct quadrangle_t *Quadrangle_Init_With_Dots(struct dot_t *dots)
{
	struct quadrangle_t *this = (struct quadrangle_t *)pvPortMalloc(sizeof(struct quadrangle_t));
	if (this == NULL)
	{
		while (1)
		{
			;
		}
	}
	this->Sort = Quadrangle_Sort;
	// this->GetDotsOnLines = Quadrangle_GetDotsOnLines;
	for (int i = 0; i < 4; i++)
	{
		this->dots[i] = dots[i];
	}
	this->Sort(this);
	this->GetSpectialDotOnLines = Quadrangle_GetSpectialDotOnLines;
	this->Equal_Scaling = Quadrangle_Equal_Scaling;
	// this->GetDotsOnLines(this);
	this->unInit = Quadrangle_unInit;

	return this;
}