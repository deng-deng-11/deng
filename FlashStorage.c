#include "FlashStorage.h"
#include "zf_common_debug.h"

extern flash_data_union flash_union_buffer[EEPROM_PAGE_LENGTH];
extern void flash_erase_page(uint32 sector_num, uint32 page_num);
extern void flash_buffer_clear(void);
extern uint8 flash_check(uint32 sector_num, uint32 page_num);

void Flash_Init(void)
{
    flash_buffer_clear();
}

void Flash_SavePathPoints(const float* x_array, const float* y_array, uint16_t num_points)
{
    uint16_t index;
    uint16_t i;
    uint16_t page;
    uint16_t points_per_page;
    uint16_t max_points = FLASH_MAX_POINTS;

    if(num_points > max_points) {
        num_points = max_points;
    }

    if(num_points == 0) {
        return;
    }

    for(page = 0; page < FLASH_PAGES; page++) {
        flash_erase_page(0, FLASH_PAGE_START + page);
    }

    points_per_page = FLASH_POINTS_PER_PAGE;

    index = 0;
    flash_union_buffer[index++].uint32_type = (uint32_t)num_points;
    flash_union_buffer[index++].uint32_type = 1U;

    for(i = 0; i < num_points && i < points_per_page; i++) {
        flash_union_buffer[index++].float_type = x_array[i];
    }
    flash_write_page(0, FLASH_PAGE_START, (uint32*)flash_union_buffer, (uint16_t)index);

    flash_buffer_clear();
    index = 0;
    for(i = 0; i < num_points && i < points_per_page; i++) {
        flash_union_buffer[index++].float_type = y_array[i];
    }
    if(index > 0) {
        flash_write_page(0, FLASH_PAGE_START + 1, (uint32*)flash_union_buffer, (uint16_t)index);
    }

    if(num_points > points_per_page) {
        flash_buffer_clear();
        index = 0;
        for(i = points_per_page; i < num_points; i++) {
            flash_union_buffer[index++].float_type = x_array[i];
        }
        if(index > 0) {
            flash_write_page(0, FLASH_PAGE_START + 2, (uint32*)flash_union_buffer, (uint16_t)index);
        }

        flash_buffer_clear();
        index = 0;
        for(i = points_per_page; i < num_points; i++) {
            flash_union_buffer[index++].float_type = y_array[i];
        }
        if(index > 0) {
            flash_write_page(0, FLASH_PAGE_START + 3, (uint32*)flash_union_buffer, (uint16_t)index);
        }
    }
}

uint16_t Flash_LoadPathPoints(float* x_array, float* y_array, uint16_t max_points)
{
    uint16_t index;
    uint16_t num_points = 0;
    uint16_t i;
    uint16_t points_per_page = FLASH_POINTS_PER_PAGE;

    flash_read_page(0, FLASH_PAGE_START, (uint32*)flash_union_buffer, EEPROM_PAGE_LENGTH);

    index = 0;
    num_points = (uint16_t)flash_union_buffer[index++].uint32_type;
    uint32_t version = flash_union_buffer[index++].uint32_type;

    if(num_points == 0) {
        return 0;
    }
    if(version != 1U) {
        return 0;
    }
    if(num_points > max_points) {
        num_points = max_points;
    }

    for(i = 0; i < num_points && i < points_per_page; i++) {
        x_array[i] = flash_union_buffer[index++].float_type;
    }

    flash_read_page(0, FLASH_PAGE_START + 1, (uint32*)flash_union_buffer, EEPROM_PAGE_LENGTH);
    index = 0;

    for(i = 0; i < num_points && i < points_per_page; i++) {
        y_array[i] = flash_union_buffer[index++].float_type;
    }

    if(num_points > points_per_page) {
        flash_read_page(0, FLASH_PAGE_START + 2, (uint32*)flash_union_buffer, EEPROM_PAGE_LENGTH);
        index = 0;

        for(i = points_per_page; i < num_points; i++) {
            x_array[i] = flash_union_buffer[index++].float_type;
        }

        flash_read_page(0, FLASH_PAGE_START + 3, (uint32*)flash_union_buffer, EEPROM_PAGE_LENGTH);
        index = 0;

        for(i = points_per_page; i < num_points; i++) {
            y_array[i] = flash_union_buffer[index++].float_type;
        }
    }

    return num_points;
}
