#include "lab2_engine.h"

int main(void)
{
    int s, i;
    unsigned seeds[] = {
        STUDENT_ID_SEED, 333333u, 444444u
    };

    print_header();

    for (s = 0; s < 3; s++) {
        for (i = 0; i <= 20; i++) {
            double p12 = i * 0.05;
            run_simulation(4, 750.0, p12, seeds[s]);
        }
    }

    return 0;
}