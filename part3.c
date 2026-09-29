#include "lab2_engine.h"

int main(void)
{
    int s, i;
    unsigned seeds[] = {
        STUDENT_ID_SEED, 333333u, 444444u
    };
    double rates[] = {
        100, 300, 500, 700, 900, 1100,
        1300, 1500, 1700, 1850, 1950
    };

    print_header();

    for (s = 0; s < 3; s++) {
        for (i = 0; i < 11; i++) {
            run_simulation(3, rates[i], 0.0, seeds[s]);
        }
    }

    return 0;
}