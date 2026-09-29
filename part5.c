#include "lab2_engine.h"

int main(void)
{
    int s, i;
    unsigned seeds[] = {
        STUDENT_ID_SEED, 333333u, 444444u
    };
    double rates[] = {
        1, 2, 4, 6, 8, 10, 12,
        14, 16, 18, 20, 21, 22
    };

    print_header();

    for (s = 0; s < 3; s++) {
        for (i = 0; i < 13; i++) {
            run_simulation(5, rates[i], 0.0, seeds[s]);
        }
    }

    return 0;
}