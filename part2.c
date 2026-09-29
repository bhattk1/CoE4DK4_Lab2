#include "lab2_engine.h"

int main(void)
{
    int s, i;

    unsigned seeds[] = {
        STUDENT_ID_SEED,
        333333u,
        444444u
    };

    double rates[] = {
        400, 402, 404, 406,
        408, 410, 415, 420
    };

    print_header();

    for (s = 0; s < 3; s++) {
        for (i = 0; i < 8; i++) {
            run_simulation(
                2,
                rates[i],
                0.0,
                seeds[s]
            );
        }
    }

    return 0;
}