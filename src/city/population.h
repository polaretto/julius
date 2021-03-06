#ifndef CITY_POPULATION_H
#define CITY_POPULATION_H

/**
 * Add people to the city.
 * @param num_people Number of people to add
 */
void city_population_add(int num_people);

/**
 * Add people returning after becoming homeless.
 * @param num_people Number of people to add
 */
void city_population_add_homeless(int num_people);

/**
 * Remove people from the city.
 * @param num_people Number of people to remove
 */
void city_population_remove(int num_people);

void city_population_remove_homeless(int num_people);

void city_population_remove_home_removed(int num_people);

void city_population_remove_for_troop_request(int num_people);

int city_population_people_of_working_age();

int city_population_number_of_school_children();

int city_population_number_of_academy_children();

void city_population_record_monthly();

void city_population_request_yearly_update();

void city_population_yearly_update();

void city_population_check_consistency();

#endif // CITY_POPULATION_H
