/* file minunit_example.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "global.h"
#include "mbpfan.h"
#include "settings.h"
#include "minunit.h"

int tests_run = 0;


static const char *test_sensor_paths()
{
    t_sensors* sensors = retrieve_sensors();
    mu_assert("No sensors found", sensors != NULL);
    t_sensors* tmp = sensors;

    while(tmp != NULL) {
        mu_assert("Sensor does not have a valid path", tmp->path != NULL);

        if(tmp->path != NULL) {
            mu_assert("Sensor does not have valid temperature", tmp->temperature > 0);
        }

        tmp = tmp->next;
    }

    return 0;
}


static const char *test_fan_paths()
{
    t_sensors* sensors = retrieve_sensors();
    mu_assert("No sensors found", sensors != NULL);
    t_sensors* tmp = sensors;
    int found_fan_path = 0;

    while(tmp != NULL) {
        if(tmp->fan_output_path != NULL) {
            found_fan_path++;
        }

        tmp = tmp->next;
    }

    mu_assert("No fans found", found_fan_path != 0);
    return 0;
}

unsigned time_seed()
{
    time_t now = time ( 0 );
    unsigned char *p = (unsigned char *)&now;
    unsigned seed = 0;
    size_t i;

    for ( i = 0; i < sizeof now; i++ ) {
        seed = seed * ( UCHAR_MAX + 2U ) + p[i];
    }

    return seed;
}

// nothing better than a horrible piece of code to
// stress a little bit the CPU
int stress(int n)
{
    int f = n;

    while (f > 0) {
        while(n > 0) {
            srand ( time_seed() );
            n--;
        }

        f--;
        n = f;
    }
}

static const char *test_get_temp()
{
    t_sensors* sensors = retrieve_sensors();
    mu_assert("No sensors found", sensors != NULL);
    unsigned short temp_1 = get_temp(sensors);
    mu_assert("Invalid Global Temperature Found", temp_1 > 1 && temp_1 < 150);
    stress(2000);
    unsigned short temp_2 = get_temp(sensors);
    mu_assert("Invalid Higher temp test (if fan was already spinning high, this is not worrying)", temp_1 < temp_2);
    return 0;
}

static const char *test_config_file()
{
    FILE *f = NULL;
    Settings *settings = NULL;
    f = fopen("/etc/mbpfan.conf", "r");
    mu_assert("No config file found", f != NULL);

    if (f == NULL) {
        return 0;
    }

    settings = settings_open(f);
    fclose(f);
    mu_assert("Could not read settings from config file", settings != NULL);

    if (settings == NULL) {
        return 0;
    }

    mu_assert("Could not read min_fan_speed from config file",settings_get_int(settings, "general", "min_fan_speed") != 0);
    mu_assert("Could not read max_fan_speed from config file",settings_get_int(settings, "general", "max_fan_speed") != 0);
    mu_assert("Could not read low_temp from config file",settings_get_int(settings, "general", "low_temp") != 0);
    mu_assert("Could not read high_temp from config file",settings_get_int(settings, "general", "high_temp") != 0);
    mu_assert("Could not read max_temp from config file",settings_get_int(settings, "general", "max_temp") != 0);
    mu_assert("Could not read polling_interval from config file",settings_get_int(settings, "general", "polling_interval") != 0);
    /* Destroy the settings object */
    settings_delete(settings);

    return 0;
}


static const char *all_tests()
{
    mu_run_test(test_sensor_paths);
    mu_run_test(test_fan_paths);
    mu_run_test(test_get_temp);
    mu_run_test(test_config_file);
    return 0;
}

int tests()
{
    printf("Starting the tests..\n");
    printf("It is normal for them to take a bit to finish.\n");
    const char *result = all_tests();

    if (result != 0) {
        printf("%s \n", result);

    } else {
        printf("ALL TESTS PASSED\n");
    }

    printf("Tests run: %d\n", tests_run);

    return result != 0;
}