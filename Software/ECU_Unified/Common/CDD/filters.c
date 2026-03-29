#include "filters.h"

long first_order_filter(long input, long prev_output, float alpha)
{
    // Fórmula do filtro de primeira ordem: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
    return alpha * input + (1 - alpha) * (prev_output);;
}
