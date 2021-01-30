#include <3ds.h>
#include <string>

#include "headers.hpp"

std::string swkbd_hint_text;
std::string swkbd_init_text;
SwkbdStatusData swkbd_state;
SwkbdLearningData swkbd_learn_data;
SwkbdDictWord swkbd_words[SWKBD_MAX_DIC_WORDS];
SwkbdButton swkbd_press_button;
SwkbdState swkbd;

void Swkbd_set_dic_word(std::string first_spell[], std::string full_spell[], int num_of_word)
{
	if(num_of_word <= SWKBD_MAX_DIC_WORDS)
	{
		for(int i = 0; i < num_of_word; i++)
			swkbdSetDictWord(&swkbd_words[i], first_spell[i].c_str(), full_spell[i].c_str());

		swkbdSetDictionary(&swkbd, swkbd_words, num_of_word);
	}
}

void Swkbd_set_parameter(SwkbdType type, SwkbdValidInput valid_type, u32 feature, u32 feature_2, int num_of_button, int max_length, std::string hint_text, std::string init_text)
{
	swkbd_hint_text = hint_text;
	swkbd_init_text = init_text;
	swkbdInit(&swkbd, type, num_of_button, max_length);
	swkbdSetHintText(&swkbd, swkbd_hint_text.c_str());
	swkbdSetValidation(&swkbd, valid_type, 0, 0);

	if(feature != (u32)-1 && feature_2 != (u32)-1)
		swkbdSetFeatures(&swkbd, feature | feature_2);
	else if(feature != (u32)-1)
		swkbdSetFeatures(&swkbd, feature);
	else if(feature_2 != (u32)-1)
		swkbdSetFeatures(&swkbd, feature_2);

	swkbdSetInitialText(&swkbd, swkbd_init_text.c_str());
	swkbdSetStatusData(&swkbd, &swkbd_state, true, true);
	swkbdSetLearningData(&swkbd, &swkbd_learn_data, true, true);
}

bool Swkbd_launch(int max_length, std::string* out_data, SwkbdButton expect_press)
{
	char swkb_input_text[max_length];
	SwkbdButton press_button;
	memset(swkb_input_text, 0x0, max_length);
	press_button = swkbdInputText(&swkbd, swkb_input_text, max_length);
	*out_data = swkb_input_text;

	if(expect_press == press_button)
		return true;
	else
		return false;
}
