#include <string.h>
#include <pulse/pulseaudio.h>
#include <stdio.h>

#include "device.h"

static struct list_devices *g_input; // struct array
static int input_idx;
static int input_num; // size of struct array

static void context_state_callback(pa_context *c, void *userdata)
{
	int *ready = userdata;

	switch(pa_context_get_state(c)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;

		case PA_CONTEXT_READY: {
			*ready = 1;
			break;
		}

		case PA_CONTEXT_TERMINATED:
		case PA_CONTEXT_FAILED:
		default: {
			*ready = 2;
			// error
			break;
		}
	}
}

static void context_sourcelist_callback(pa_context *c,
	const pa_source_info *i, int eol, void *userdata)
{
	// end of list
	if(eol > 0)
		return;

	if(input_idx >= input_num)
	{
		int i;
		struct list_devices *tmp = malloc(
			sizeof(struct list_devices) * (input_num+1));

		for(i = 0; i < input_num; i++) {
			tmp[i].name = g_input[i].name;
			tmp[i].description = g_input[i].description;
		}
		input_num++;
		free(g_input);
		g_input = tmp; 
	}
	g_input[input_idx].name
		= malloc((strlen(i->name)+1)*sizeof(char));
	strcpy(g_input[input_idx].name, i->name);

	g_input[input_idx].description
		= malloc((strlen(i->description)+1)*sizeof(char));
	strcpy(g_input[input_idx].description, i->description);
	
	input_idx++;
}

struct list_devices *getInputDeviceList(int *len)
{
	pa_mainloop *ml;
	pa_mainloop_api *ml_api;
	pa_context *context;
	pa_operation *operation;
	int state = 0;
	int ready = 0;

	if(!(ml = pa_mainloop_new()))
		return NULL;
	ml_api = pa_mainloop_get_api(ml);
	context = pa_context_new(ml_api, NULL);

	// Connect the context
	if(pa_context_connect(context, NULL, 0, NULL) < 0)
		return NULL;
	pa_context_set_state_callback(context, context_state_callback, &ready);

	input_idx = 0;
	input_num = 3;
	g_input = malloc(
		sizeof(struct list_devices) * input_num);

	for(;;) {
		if(ready == 0) {
			pa_mainloop_iterate(ml, 1, NULL);
			continue;
		}
		if(ready == 2) {
			pa_context_disconnect(context);
			pa_context_unref(context);
			pa_mainloop_free(ml);
			return NULL;
		}

		switch(state) {
			case 0: {
				operation = pa_context_get_source_info_list(
					context, context_sourcelist_callback, NULL);

				state++;
				break;
			}
			case 1: {
				if(pa_operation_get_state(operation) == PA_OPERATION_DONE) {
					pa_operation_unref(operation);
					pa_context_disconnect(context);
					pa_context_unref(context);
					pa_mainloop_free(ml);
					*len = input_num;
					return g_input;
				}
				break;
			}
			default: {
				pa_mainloop_free(ml);
				return NULL;
			}
		}
		pa_mainloop_iterate(ml, 1, NULL);
	}
}

void freeDeviceList(struct list_devices *list, int len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		free(list[i].name);
		free(list[i].description);
	}
	free(list);
}

