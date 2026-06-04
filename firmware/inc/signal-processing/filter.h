#pragma once

/**
 * @brief Possible filters that can be applied to modify the signal.
 */
typedef enum {
    passthrough,
    noiseSuppression,
    customEqualized,
} Filter;

/**
 * @brief Instance of filter
 */
extern Filter filter;

/**
 * @brief Selects one of the filters.
 * @param filter The new filter to be applied.
 */
void setFilter(Filter filter);

/**
 * @brief Clear the filter.
 */
void clearFilter(void);

/**
 * @brief Execute the filter.
 */
void executeFilter(void);