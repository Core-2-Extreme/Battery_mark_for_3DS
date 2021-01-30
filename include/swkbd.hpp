#pragma once

#define SWKBD_MAX_DIC_WORDS 16

#define SWKBD_TOO_MANY_DIC_WORDS 78593

void Swkbd_set_dic_word(std::string first_spell[], std::string full_spell[], int num_of_word);

void Swkbd_set_parameter(SwkbdType type, SwkbdValidInput valid_type, u32 feature, u32 feature_2, int num_of_button, int max_length, std::string hint_text, std::string init_text);

bool Swkbd_launch(int max_length, std::string* out_data, SwkbdButton expect_press);
