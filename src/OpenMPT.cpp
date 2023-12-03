#include "OpenMPT.hpp"
#include <libopenmpt/libopenmpt.hpp>

void OpenMPT::_register_methods() {
	godot::register_method("set_audio_generator_playback", &OpenMPT::set_audio_generator_playback);
	godot::register_method("_fill_thread_func",  &OpenMPT::_fill_thread_func);
	godot::register_method("is_module_loaded",  &OpenMPT::is_module_loaded);
	godot::register_method("get_cell", &OpenMPT::get_cell);
	godot::register_method("get_current_cell", &OpenMPT::get_current_cell);
	godot::register_method("get_cell_command", &OpenMPT::get_cell_command);
	godot::register_method("get_current_cell_command", &OpenMPT::get_current_cell_command);
	godot::register_method("get_pattern_commands", &OpenMPT::get_pattern_commands);
	godot::register_method("cell_is_empty", &OpenMPT::cell_is_empty);
	godot::register_method("set_repeat_count", &OpenMPT::set_repeat_count);
	godot::register_method("get_duration_seconds", &OpenMPT::get_duration_seconds);
	godot::register_method("get_row_string", &OpenMPT::get_row_string);
	godot::register_method("get_channel_string", &OpenMPT::get_channel_string);
	godot::register_method("get_pattern_num_rows", &OpenMPT::get_pattern_num_rows);
	godot::register_method("get_position_seconds", &OpenMPT::get_position_seconds);
	godot::register_method("set_position_seconds", &OpenMPT::set_position_seconds);
	godot::register_method("get_current_estimated_bpm", &OpenMPT::get_current_estimated_bpm);
	godot::register_method("get_current_speed", &OpenMPT::get_current_speed);
	godot::register_method("get_current_tempo", &OpenMPT::get_current_tempo);
	godot::register_method("get_current_order", &OpenMPT::get_current_order);
	godot::register_method("get_current_row", &OpenMPT::get_current_row);
	godot::register_method("get_current_pattern", &OpenMPT::get_current_pattern);
	godot::register_method("get_current_playing_channels", &OpenMPT::get_current_playing_channels);
	godot::register_method("get_current_channel_volume", &OpenMPT::get_current_channel_volume);
	godot::register_method("get_current_channel_volume_left", &OpenMPT::get_current_channel_volume_left);
	godot::register_method("get_current_channel_volume_right", &OpenMPT::get_current_channel_volume_right);
	godot::register_method("get_num_subsongs", &OpenMPT::get_num_subsongs);
	godot::register_method("get_num_channels", &OpenMPT::get_num_channels);
	godot::register_method("get_num_patterns", &OpenMPT::get_num_patterns);
	godot::register_method("get_num_orders", &OpenMPT::get_num_orders);
	godot::register_method("get_num_instruments", &OpenMPT::get_num_instruments);
	godot::register_method("get_num_samples", &OpenMPT::get_num_samples);
	godot::register_method("get_subsong_names", &OpenMPT::get_subsong_names);
	godot::register_method("get_channel_names", &OpenMPT::get_channel_names);
	godot::register_method("get_order_names", &OpenMPT::get_order_names);
	godot::register_method("get_pattern_names", &OpenMPT::get_pattern_names);
	godot::register_method("get_instrument_names", &OpenMPT::get_instrument_names);
	godot::register_method("get_sample_names", &OpenMPT::get_sample_names);
	godot::register_method("get_order_pattern", &OpenMPT::get_order_pattern);
	godot::register_method("get_metadata", &OpenMPT::get_metadata);
	godot::register_method("get_metadata_keys", &OpenMPT::get_metadata_keys);
	godot::register_method("start", &OpenMPT::start);
	godot::register_method("stop", &OpenMPT::stop);
	//godot::register_signal("")

	godot::register_property<OpenMPT, godot::String>(
		"module_file_path",
		&OpenMPT::set_module_file_path,
		&OpenMPT::get_module_file_path,
		"",
		GODOT_METHOD_RPC_MODE_DISABLED,
		GODOT_PROPERTY_USAGE_DEFAULT,
		GODOT_PROPERTY_HINT_GLOBAL_FILE);
}

void OpenMPT::_init() {
	// Might want to remove this
	PRINT_MESSAGE("Initializing");
	PRINT_MESSAGE("libopenmpt version: core " + get_core_version() + " lib "+ get_library_version());
}

void OpenMPT::load_module_data() {
	if (module_file_path.empty()) {
		return;
	}

	PRINT_MESSAGE("Loading: " + module_file_path);
	if (module_file) {
		PRINT_ERROR("BUG: Module file handle should be null on init.");
		return;
	}

	module_file = godot::File::_new();
	godot::Error err = module_file->open(module_file_path, godot::File::ModeFlags::READ);

	if (err != godot::Error::OK) {
		PRINT_ERROR(godot::String("Could not open '") + module_file_path + "' for reading.");
		module_file->close();
		module_file->free();
		module_file = nullptr;
		return;
	}

	godot::PoolByteArray bytes = module_file->get_buffer(module_file->get_len());

	try {
		if (module) {
			delete module;
			module = nullptr;
		}
		module = new openmpt::module(bytes.read().ptr(), bytes.size());
		set_repeat_count(-1);
	} catch (openmpt::exception e) {
		PRINT_ERROR(godot::String("Could not initialize OpenMPT module: ") + e.what());
		module_file->close();
		module_file->free();
		module_file = nullptr;
		return;
	}

	PRINT_MESSAGE("Load Finished");

	// Apparently you shouldnt call delete on it and instead call member
	// funcs for freeing
	module_file->close();
	module_file->free();
	module_file = nullptr;
}

godot::String OpenMPT::get_cell(int pattern, int row, int channel) {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return godot::String();
	}
	return module->format_pattern_row_channel(pattern, row, channel).c_str();
}

godot::String OpenMPT::get_current_cell(int channel) {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return godot::String();
	}
	return module->format_pattern_row_channel(
		module->get_current_pattern(),
		module->get_current_row(),
		channel).c_str();
}

godot::Dictionary OpenMPT::get_pattern_commands() {
	godot::Dictionary c;

	c["note"] = openmpt::module::command_index::command_note;
	c["instrument"] = openmpt::module::command_index::command_instrument;
	c["volumeffect"] = openmpt::module::command_index::command_volumeffect;
	c["volume"] = openmpt::module::command_index::command_volume;
	c["effect"] = openmpt::module::command_index::command_effect;

	return c;
}

godot::String OpenMPT::get_cell_command(int pattern, int row, int channel, int command) {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return -1;
	}
	return module->format_pattern_row_channel_command(pattern, row, channel, command).c_str();
}

godot::String OpenMPT::get_current_cell_command(int channel, int command) {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return -1;
	}
	return module->format_pattern_row_channel_command(
		module->get_current_pattern(),
		module->get_current_row(),
		channel, command).c_str();
}



bool OpenMPT::cell_is_empty(int pattern, int row, int channel) {
	return module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_note) == 0 &&
		   module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_instrument) == 0 &&
		   module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_effect) == 0 &&
		   module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_volumeffect) == 0 &&
		   module->get_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_volume) == 0;
}


int64_t OpenMPT::get_current_row() {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return -1;
	}

	return module->get_current_row();
}

int64_t OpenMPT::get_current_pattern() {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return -1;
	}

	return module->get_current_pattern();
}

godot::String OpenMPT::get_row_string(int pattern, int row) {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return -1;
	}

	godot::String rstr;

	for (int i = 0; i < module->get_num_channels(); i++) {
		rstr += module->format_pattern_row_channel(pattern, row, i).c_str();
		rstr += " ";
	}

	return rstr;
}

godot::String OpenMPT::get_channel_string(int pattern, int channel) {
	if (!module) {
		PRINT_ERROR("No Module Loaded");
		return -1;
	}

	godot::String rstr;

	for (int i = 0; i < module->get_pattern_num_rows(pattern); i++) {
		rstr += module->format_pattern_row_channel(pattern, i, channel).c_str();
		rstr += "\n";
	}

	return rstr;
}

