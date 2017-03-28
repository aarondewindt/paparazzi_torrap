//
// Created by elviento on 3/26/17.
//

#ifndef TORRAP_TORRAP_DIRECTION_DETECTION_C_H
#define TORRAP_TORRAP_DIRECTION_DETECTION_C_H

//
// Created by elviento on 3/22/17.
//

#ifndef TORRAP_TORRAP_H
#define TORRAP_TORRAP_H

#include "modules/computer_vision/lib/vision/image.h"
//#include "types.h"

struct ColorFilter {
    uint8_t y_m;
    uint8_t y_M;
    uint8_t u_m;
    uint8_t u_M;
    uint8_t v_m;
    uint8_t v_M;

    float y_start;
    float y_end;
    float threshold;
    float gain;
    float turn_gain;
};

struct ImageSection {
    uint16_t start;
    uint16_t end;
    uint32_t size;
    float score;
    float turn_score;
    float gain;
};

struct TorrapDirectionResult {
    int8_t direction;
    int8_t panic;
    int8_t turn;
    struct image_t * img;
};

extern struct ColorFilter orange_filter;
extern struct ColorFilter green_filter;
extern struct ColorFilter black_filter;
extern struct ColorFilter blue_filter;
extern float turn_threshold;


struct TorrapDirectionResult* torrap_direction_detection(struct image_t *img);
void torrap_init_sections(struct image_t *img);
float calc_gain(uint16_t width, uint16_t loc);
float calc_direction();
void reset_sections();

void color_section_score_update(struct image_t *img,
                                struct ColorFilter *filter_info );

uint16_t torrap_yuv422_colorfilt(struct image_t *input,
                                 struct ColorFilter *filter_info);

void configure_output_image(struct image_t *img);

void print_status();

#endif //TORRAP_TORRAP_H


#endif //TORRAP_TORRAP_DIRECTION_DETECTION_C_H
