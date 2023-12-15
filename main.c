#include "wayland-client-core.h"
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include "treeland-output-management-client-protocol.h"

struct output_state {
	struct treeland_output_manager_v1 *output_manager;

	struct wl_list heads;
	char *primary_output;
	uint32_t serial;
	bool has_serial;
	bool running;
	bool failed;
};

void output_manager_handle_primary_output(void *data,
			       struct treeland_output_manager_v1 *treeland_output_manager_v1,
			       const char *output_name) {
	printf("%s", output_name);
}

static const struct treeland_output_manager_v1_listener output_manager_listener = {
	.primary_output = output_manager_handle_primary_output
};

static void registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	struct output_state *state = data;

	if (strcmp(interface, treeland_output_manager_v1_interface.name) == 0) {
		uint32_t version_to_bind = version <= 1 ? version : 1;
		state->output_manager = wl_registry_bind(registry, name,
			&treeland_output_manager_v1_interface, version_to_bind);
		treeland_output_manager_v1_add_listener(state->output_manager,
			&output_manager_listener, state);
		printf("interface: '%s', version: %d, name: %d\n",
            interface, version, name);
	}
}

static void registry_handle_global_remove(void *data,
		struct wl_registry *registry, uint32_t name) {
	// This space is intentionally left blank
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

int main(int argc, char *argv[]) {
	struct output_state state = { .running = true };
	//wl_list_init(&state.heads);

	// connect to wayland display
	struct wl_display *display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "failed to connect to display\n");
		return EXIT_FAILURE;
	}

	// get handle on the registry
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, &state);

	// send our messages (async), does this happen anyway??
	// if (wl_display_dispatch(display) < 0) {
	// 	fprintf(stderr, "wl_display_dispatch failed\n");
	// 	return EXIT_FAILURE;
    // }

	// block here because we want to wait for our async reply of the server
	if (wl_display_roundtrip(display) < 0) {
		fprintf(stderr, "wl_display_roundtrip failed\n");
		return EXIT_FAILURE;
	}

	if (state.output_manager == NULL) {
		fprintf(stderr, "compositor doesn't support "
			"wlr-output-management-unstable-v1\n");
		return EXIT_FAILURE;
	}

	//while (!state.has_serial) {
		// if (wl_display_dispatch(display) < 0) {
		// 	fprintf(stderr, "wl_display_dispatch failed\n");
		// 	return EXIT_FAILURE;
		// }
	//}

	treeland_output_manager_v1_set_primary_output(state.output_manager, "output-wayland");

	if (wl_display_dispatch(display) != -1) {
		// This space intentionally left blank
	}
	wl_display_flush(display);


	treeland_output_manager_v1_destroy(state.output_manager);
	wl_registry_destroy(registry);
	wl_display_disconnect(display);

	return state.failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
