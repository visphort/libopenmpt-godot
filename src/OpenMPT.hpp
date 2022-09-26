#ifndef SIMPLE_H
#define SIMPLE_H

#include <Godot.hpp>
#include <Input.hpp>
#include <File.hpp>
#include <Reference.hpp>
#include <Sprite.hpp>
#include <Ref.hpp>
#include <AudioStreamPlayer.hpp>
#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>
#include <Thread.hpp>

#include <PoolArrays.hpp>
#include <bits/stdint-intn.h>
#include <bits/types/__FILE.h>
#include <libopenmpt/libopenmpt.hpp>
#include <string>
#include <vector>
#include <stdio.h>

#define PRINT_ERROR(desc) (godot::Godot::print_error((desc), __func__, __FILE__, __LINE__))
#define PRINT_MESSAGE(msg) (godot::Godot::print("[libopenmpt-godot]: " msg))

#define MOD_NULL_CHECK {\
if (!module) { \
	PRINT_ERROR("No Module Loaded"); \
	return -1; \
} \
}

#define MOD_NULL_CHECK_VOID {\
if (!module) { \
	PRINT_ERROR("No Module Loaded"); \
	return; \
} \
}

class OpenMPT : public godot::Node {
	GODOT_CLASS(OpenMPT, godot::Reference)

	godot::String data;
	godot::Ref<godot::AudioStreamGeneratorPlayback> audgen;
	int sample_rate = 0;
	openmpt::module *module = nullptr;
	godot::File *module_file = nullptr;
	int module_log;
	godot::String module_file_path;
	int data2;

	// godot_real is a float, so it should work as a float buffer for our
	// purposes
	godot::PoolRealArray lbuf;
	godot::PoolRealArray rbuf;
	godot::PoolVector2Array pushbuf;
	int bufsize = 0;

	godot::Thread *fill_thread;
	bool continue_fill_thread = false;

public:
	~OpenMPT() {
		PRINT_MESSAGE("Exiting");
		if (fill_thread) {
			if (fill_thread->is_alive()) {
				PRINT_MESSAGE("Finishing thread");
				continue_fill_thread = false;
				fill_thread->wait_to_finish();
				PRINT_MESSAGE("Thread finished");
			}
			PRINT_MESSAGE("Freeing");
			fill_thread->free();
			fill_thread = nullptr;
		}
		audgen->unreference();
		delete module;
		module = nullptr;
	}

	static inline uint8_t get_byte(int v, int n) {
		return (v >> (n * 8)) & 0xff;
	}

	static godot::String get_core_version() {
		int v = openmpt::get_core_version();
		char buf[32];
		snprintf(buf, 32, "%d.%d.%d.%d", get_byte(v, 3), get_byte(v, 2), get_byte(v, 1), get_byte(v, 0));
		return godot::String(buf);
	}

	static godot::String get_library_version() {
		int v = openmpt::get_core_version();
		char buf[32];
		snprintf(buf, 32, "%d.%d.%d.%d", get_byte(v, 3), get_byte(v, 2), get_byte(v, 1), get_byte(v, 0));
		return godot::String(buf);
	}


	static void _register_methods();

	void _init();

	void load_module_data();

	bool is_module_loaded() {
		return module != nullptr;
	}

	void set_module_file_path(godot::String val) {
		module_file_path = val;
		load_module_data();
	}

	godot::String get_module_file_path() {
		return module_file_path;
	}

	godot::String get_cell(int pattern, int row, int channel);
	godot::String get_current_cell(int channel);
	godot::String get_cell_command(int pattern, int row, int channel, int command);
	godot::String get_current_cell_command(int channel, int command);
	godot::Dictionary get_pattern_commands();
	bool cell_is_empty(int pattern, int row, int channel);


	void set_repeat_count(int repeat_count) {
		MOD_NULL_CHECK_VOID
		module->set_repeat_count(repeat_count);
	}

	double get_duration_seconds() {
		MOD_NULL_CHECK
		return module->get_duration_seconds();
	}

	godot::String get_row_string(int pattern, int row);

	godot::String get_channel_string(int pattern, int channel);


	int64_t get_pattern_num_rows(int pattern) {
		MOD_NULL_CHECK
		return module->get_pattern_num_rows(pattern);
	}

	double get_position_seconds() {
		MOD_NULL_CHECK
		return module->get_position_seconds();
	}

	void set_position_seconds(double seconds) {
		MOD_NULL_CHECK_VOID
		module->set_position_seconds(seconds);
	}

	double get_current_estimated_bpm() {
		MOD_NULL_CHECK
		return module->get_current_estimated_bpm();
	}

	int64_t get_current_speed() {
		MOD_NULL_CHECK
		return module->get_current_speed();
	}

	int64_t get_current_tempo() {
		MOD_NULL_CHECK
		return module->get_current_tempo();
	}

	int64_t get_current_order() {
		MOD_NULL_CHECK
		return module->get_current_order();
	}

	double set_position_order_row(int order, int row) {
		MOD_NULL_CHECK
		return module->set_position_order_row(order, row);
	}

	int64_t get_current_row();

	int64_t get_current_pattern();

	int64_t get_current_playing_channels() {
		MOD_NULL_CHECK
		return module->get_current_playing_channels();
	}

	double get_current_channel_volume(int channel) {
		MOD_NULL_CHECK
		return module->get_current_channel_vu_mono(channel);
	}

	double get_current_channel_volume_left(int channel) {
		MOD_NULL_CHECK
		return module->get_current_channel_vu_left(channel);
	}

	double get_current_channel_volume_right(int channel) {
		MOD_NULL_CHECK
		return module->get_current_channel_vu_left(channel);
	}

	int64_t get_num_subsongs() {
		MOD_NULL_CHECK
		return module->get_num_subsongs();
	}

	int64_t get_num_channels() {
		MOD_NULL_CHECK
		return module->get_num_channels();
	}

	int64_t get_num_patterns() {
		MOD_NULL_CHECK
		return module->get_num_patterns();
	}

	int64_t get_num_orders() {
		MOD_NULL_CHECK
		return module->get_num_orders();
	}

	int64_t get_num_instruments() {
		MOD_NULL_CHECK
		return module->get_num_instruments();
	}

	int64_t get_num_samples() {
		MOD_NULL_CHECK
		return module->get_num_instruments();
	}

	godot::PoolStringArray strvec2godot(std::vector<std::string> v) {
		godot::PoolStringArray p;
		for (int i = 0; i < v.size(); i++) {
			p.append(godot::String(v[i].c_str()));
		}
		return p;
	}

	godot::PoolStringArray get_subsong_names() {
		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return godot::PoolStringArray();
		}

		return strvec2godot(module->get_subsong_names());
	}

	godot::PoolStringArray get_channel_names() {
		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return godot::PoolStringArray();
		}

		return strvec2godot(module->get_channel_names());
	}

	godot::PoolStringArray get_order_names() {
		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return godot::PoolStringArray();
		}

		return strvec2godot(module->get_order_names());
	}

	godot::PoolStringArray get_pattern_names() {
		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return godot::PoolStringArray();
		}

		return strvec2godot(module->get_pattern_names());
	}

	godot::PoolStringArray get_instrument_names() {
		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return godot::PoolStringArray();
		}

		return strvec2godot(module->get_instrument_names());
	}

	godot::PoolStringArray get_sample_names() {
		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return godot::PoolStringArray();
		}

		return strvec2godot(module->get_sample_names());
	}

	int64_t get_order_pattern(int order) {
		MOD_NULL_CHECK
		return module->get_order_pattern(order);
	}

	godot::PoolStringArray get_metadata_keys() {
		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return godot::PoolStringArray();
		}

		return strvec2godot(module->get_metadata_keys());
	}

	godot::String get_metadata(godot::String meta) {
		MOD_NULL_CHECK
		return module->get_metadata(meta.alloc_c_string()).c_str();
	}


	void stop() {
		if (!fill_thread) {
			return;
		}

		if (fill_thread->is_alive()) {
			continue_fill_thread = false;
			fill_thread->wait_to_finish();
		}

		fill_thread->unreference();
		fill_thread->free();
		fill_thread = nullptr;
	}

	// Note: Apparently only the first call of default_argument fills in the
	// reset_position argument for us. The rest of the time this does not
	// happen and needs to be passed explicitly. Weird. Possibly because of
	// template magic not persiting the option or something but doesn't this
	// just come under polymorphism?
	// Maybe because the function isn't called anywhere in the C++ code the
	// function signature isn't generated or something like that.
	void start(bool reset_position = false) {
		if (fill_thread && fill_thread->is_alive()) {
			stop();
		}

		if (!module) {
			PRINT_ERROR("No Module Loaded");
			return;
		}

		if (reset_position) {
			module->set_position_seconds(0);
		}

		continue_fill_thread = true;

		_fill_buffer();
		fill_thread = godot::Thread::_new();
		fill_thread->reference();
		fill_thread->start(this, "_fill_thread_func");
	}

	void set_audio_generator_playback(godot::AudioStreamPlayer *a) {
		stop();

		audgen = a->get_stream_playback();
		audgen->reference();

		if (!audgen.is_valid()) {
			PRINT_ERROR("Could not load audio generator (invalid parameter)?");
		}

		godot::Ref<godot::AudioStreamGenerator> stream = a->get_stream();
		stream->reference();

		if (!stream.is_valid()) {
			PRINT_ERROR("Could not load audio stream (invalid parameter)?");
		}

		audgen->unreference();

		sample_rate = stream->get_mix_rate();
	}

	void _fill_thread_func(godot::Variant v) {
		while (continue_fill_thread) {
			_fill_buffer();
		}
		PRINT_MESSAGE("Thread Exiting");
	}

	void _fill_buffer() {
		// This really doesn't make sense to me. Why would you fill the current
		// available frame count and not up to the total buffer limit? What
		// happens when this reaches 0 on lag or something else?
		int count = audgen->get_frames_available();

		if (!audgen->can_push_buffer(count)) {
			return;
		}
		
		if (count > bufsize) {
			bufsize = count;
			lbuf.resize(bufsize);
			rbuf.resize(bufsize);
		}

		pushbuf.resize(count);

		// buf should always read count samples, not bufsize.
		int frames = module->read(sample_rate, count, lbuf.write().ptr(), rbuf.write().ptr());
		// if (frames == 0) {
		// 	// end of song
		// 	continue_fill_thread = false;
		// }
		for (int i = 0; i < count; i++) {
			pushbuf.set(i, godot::Vector2(lbuf[i], rbuf[i]));
		}
		audgen->push_buffer(pushbuf);
	}

};

#endif // SIMPLE_H
