//
// Created by elviento on 3/26/17.
//

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "modules/torrap/torrap_direction_detection.h"

#define GAIN_SCALE 1.
#define N_SECTIONS 20
#define SECTION_SPREAD 3.8

#define sign(x) (x < 0 ? -1 : 1)

struct ImageSection *sections = NULL;
struct TorrapDirectionResult result;
float turn_threshold = 0.85;

uint8_t *working_buffer = NULL;

struct image_t output_image;
uint8_t *output_buffer = NULL;


struct ColorFilter orange_filter = {
        .y_m = 20,
        .y_M = 255,
        .u_m = 75,
        .u_M = 145,
        .v_m = 167,
        .v_M = 255,
        .y_start = 0,
        .y_end = 1.,
        .threshold = 0,
        .gain = -1,
		.turn_gain = 1
};


struct ColorFilter green_filter = {
        .y_m = 20,
        .y_M = 80,
        .u_m = 0,
        .u_M = 114,
        .v_m = 0,
        .v_M = 145,
        .y_start = 0,
        .y_end = 1.,
        .threshold = 0,
        .gain = 2,
		.turn_gain = 0
};


struct ColorFilter black_filter = {
        .y_m = 0,
        .y_M = 60,
        .u_m = 120,
        .u_M = 135,
        .v_m = 110,
        .v_M = 135,
        .y_start = 0.,
        .y_end = 1.,
        .threshold = 0.,
        .gain = -1,
		.turn_gain = 1
};

struct ColorFilter blue_filter = {
        .y_m = 30,
        .y_M = 255,
        .u_m = 128,
        .u_M = 255,
        .v_m = 0,
        .v_M = 124,
        .y_start = 0,
        .y_end = 1.,
        .threshold = 0.,
        .gain = -1,
		.turn_gain = 1
};


struct TorrapDirectionResult* torrap_direction_detection(struct image_t *img)
{
    // Initialize sections if necessary.
    if (sections == NULL) {
        torrap_init_sections(img);
    }

    // Reset section scores to 0.
    reset_sections();

    color_section_score_update(img, &green_filter);
    color_section_score_update(img, &black_filter);
    color_section_score_update(img, &orange_filter);
    color_section_score_update(img, &blue_filter);

    // Calculate the direction in which to go.
    result.direction = sign(calc_direction());
    result.img = &output_image;

    print_status();

    return &result;
}

void print_status() {
	if (sections == NULL) {
		return;
	}

    printf("Direction: %d\nTurn     : %d\nScore    : ", result.direction, result.turn);
    for (int i = 0; i < N_SECTIONS; i++) {
        printf("%f ", sections[i].score);
    }

    printf("\nTurn Score: ");
    for (int i = 0; i < N_SECTIONS; i++) {
	    printf("%f ", sections[i].turn_score);
    }

    printf("\nSize: ");
	for (int i = 0; i < N_SECTIONS; i++) {
		printf("%d ", sections[i].size);
	}

    printf("\nGain     : ");
    for (int i = 0; i < N_SECTIONS; i++) {
        printf("%f ", sections[i].gain);
    }

    printf("\nStart     : ");
	for (int i = 0; i < N_SECTIONS; i++) {
		printf("%d ", sections[i].start);
	}

    printf("\n\n");
}

void configure_output_image(struct image_t *img) {

}

uint16_t torrap_section_location(uint16_t image_width, uint16_t i) {
	float q = ((float)(fabs(i - (N_SECTIONS / 2.)))) / ((float)(N_SECTIONS / 2));
	uint32_t section_location = (1 + (i < (N_SECTIONS / 2) ? -1 : 1) * pow(q, SECTION_SPREAD)) * image_width / 2 ;

	printf("location %d\n", section_location);

	return section_location;
}

void torrap_init_sections(struct image_t *img){
    output_buffer = malloc(img->w * img->h);
    working_buffer = malloc(img->w * img->h);


    // Allocate memory for the sections definitions. Loop through each section and set their size.
    sections = (struct ImageSection*)malloc(sizeof(struct ImageSection) * N_SECTIONS);
    for (uint16_t i = 0; i < N_SECTIONS; i ++){
    	// Sections initialization
//    	uint16_t width = torrap_section_width(img->h, i);

        sections[i].start = torrap_section_location(img->h, i);
        sections[i].end = torrap_section_location(img->h, i+1);
        sections[i].size = (sections[i].end - sections[i].start) * img->w;

        // Calculate the gains for each section.

        uint16_t center = (sections[i].end + sections[i].start) / 2;
        sections[i].gain = calc_gain(img->h, center);
    }

    // Normalize gains
    // Get the sum of the gains
    float gain_sum = 0;
    for (int i = 0; i < N_SECTIONS; i++) {
        gain_sum += fabs(sections[i].gain);
    }

    // Divide sum of the scores to finish normalization.
    for (int i = 0; i < N_SECTIONS; i++) {
        sections[i].gain /= gain_sum;
    }

    print_status();
}

float calc_gain(uint16_t width, uint16_t loc) {
    return (-1.f * (float)fabs(loc - (width / 2.f)) / width * 2.f + 1.f) * ((loc < (width / 2.f)) ? -1.f : 1.f);;
}

float calc_direction() {
    float direction = 0;

    // Normalize scores so that the sum of all scores equals 1.
    // Get the sum of the scores
    float score_sum = 0;
    for (int i = 0; i < N_SECTIONS; i++) {
    	if (sections[i].turn_score > turn_threshold) {
    		result.turn = 1;
    	}
        score_sum += fabs(sections[i].score);
    }

    // If the sum of the scores is exactly 0. PANIC.
    // TODO: Add something somewhere to handle the panic.
    if (score_sum == 0.0) {
        result.panic = 1;
        return 0;
    } else{
        result.panic = 0;
    }

    // Divide sum of the scores to finish normalization.
    for (int i = 0; i < N_SECTIONS; i++) {
        sections[i].score /= score_sum;
    }

    // Apply first gain
    for (int i = 0; i < N_SECTIONS; i++) {
        direction += GAIN_SCALE * sections[i].gain * sections[i].score;
    }

    return direction;
}

void reset_sections() {
    // Reset all section scores to 0.
    for (uint16_t i = 0; i < N_SECTIONS; ++i) {
        sections[i].score = 0;
        sections[i].turn_score = 0;
    }

    result.turn = 0;
}


void color_section_score_update(struct image_t *img,
                                struct ColorFilter *filter_info ){
    uint32_t count = 0;
    memset(working_buffer, 0, img->w * img->h);

    // Apply color filter, the result will be stored in the working_buffer for further processing.
    int image_count = torrap_yuv422_colorfilt(img, filter_info);
    // printf("image_count %d\n", image_count);

    uint32_t j = 0;

    // Loop through each section.
    for (uint16_t i = 0; i < N_SECTIONS; i++){
        count = 0;

        // Loop through each pixel in the section
        for (uint16_t y = sections[i].start; y < sections[i].end; y++){
            for (uint16_t x = 0; x < img->w; x++) {

                // If the value of the pixel is 255, increase the counter.
                if (working_buffer[j++] == 255) {
                    count++;
                }

                if ((y == sections[i].start) && ((x%2)==0))
                {
                	uint8_t *pixels = (uint8_t*)img->buf + (y * img->w + x) * 2;
                	pixels[0] = 127;
                	pixels[1] = 255;
                	pixels[2] = 127;
                	pixels[3] = 255;
                }
            }
        }

        float size = sections[i].size * (filter_info->y_end - filter_info->y_start);

        // Only add to the score if the minimum threshold is satisfied.
        if (count >= (size * filter_info->threshold)) {
            sections[i].score += (float)count * filter_info->gain;  // / size
            sections[i].turn_score += (float)count / size * filter_info->turn_gain;
        }
    }
}

/**
 * Filter colors in an YUV422 image
 * @param[in] *input The input image to filter
 * @param[out] *output The filtered output image
 * @return The amount of filtered pixels
 */
uint16_t torrap_yuv422_colorfilt(struct image_t *input,
                                 struct ColorFilter *filter_info)
{
//	printf("hw %d %d", input->h, input->w);
    uint64_t start_offset = (uint64_t )(filter_info->y_start * input->w);
    uint16_t cnt = 0;
    uint8_t *source = input->buf + start_offset * 2;
    uint8_t *dest = working_buffer + start_offset;

    // Go trough all the pixels
    for (uint16_t y = 0;
         y < input->h; y++) {

        for (uint16_t x = (uint16_t )(input->w * filter_info->y_start);
        		x < (uint16_t)(input->w * filter_info->y_end); x += 2) {

        	if (source[1] != 255) {
				// Check if the color is inside the specified values
				if (
						(source[1] >= filter_info->y_m)
						&& (source[1] <= filter_info->y_M)
						&& (source[0] >= filter_info->u_m)
						&& (source[0] <= filter_info->u_M)
						&& (source[2] >= filter_info->v_m)
						&& (source[2] <= filter_info->v_M)
						) {

					cnt ++;
					dest[0] = 255;
					dest[1] = 255;
					// If it's black, make it pink. Otherwise the detected color.
					if (filter_info->y_m == 0) {
						source[0] = 255;
						source[1] = 255;
						source[2] = 255;
						source[3] = 255;
					} else{
						source[0] = filter_info->u_m < 127 ? filter_info->u_m : filter_info->u_M;
						source[1] = 255;
						source[2] = filter_info->v_m < 127 ? filter_info->v_m : filter_info->v_M;
						source[3] = 255;
					}


				} else {
					dest[0] = source[1];
					dest[1] = source[3];
				}
        	}

//            source[0] = 127;
//            source[2] = 127;

            // Go to the next 2 pixels
            dest += 2;
            source += 4;
        }
        uint32_t n_skip_pixels = (uint32_t)(((filter_info->y_start + (1 - filter_info->y_end)) * input->w));
        source += n_skip_pixels * 2;
        dest += n_skip_pixels;

    }

    return cnt;
}
