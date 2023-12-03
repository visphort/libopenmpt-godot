# Pretty shoddily written. Main purpose is to show how the GDNative library works.

extends Control

onready var module = $module
onready var generator_player = $generator_player
onready var patternscroller = $patternscroller
onready var container = $patternscroller/container
onready var tween = $Tween
onready var info = $playback_controls/container/info
onready var seek = $playback_controls/container/set_seek
onready var speed = $playback_controls/container/set_speed
onready var playing = $playback_controls/container/playing
onready var file_name = $playback_controls/container/container/file_name
onready var volume = $playback_controls/container/volume

onready var defaultpos = -get_viewport_rect().size.y / 8

var spectrum: AudioEffectSpectrumAnalyzerInstance
var labels: Array = []
var prev_pattern: int = -1
var prev_row: int = -1
var playing_channels: int = 0

var update_seek: bool = true

func _input(event: InputEvent) -> void:
	if event is InputEventMouseMotion:
		var offset = patternscroller.rect_size.x - get_viewport_rect().size.x
		if patternscroller.rect_size.x - get_viewport_rect().size.x > 0:
			patternscroller.rect_position.x = -offset * get_global_mouse_position().x / get_viewport_rect().size.x


func set_metadata() -> void:
	var vals: PoolStringArray = module.get_metadata_keys()

	$metadata/container/type.text = \
	str(module.get_metadata("type"), " (", module.get_metadata("type_long"), ")")

	if module.get_metadata("originaltype") != "":
		$metadata/container/originaltype.text = \
		str(module.get_metadata("originaltype"), " (", module.get_metadata("originaltype_long"), ")")
	else:
		$metadata/container/originaltype.text = ""

	if module.get_metadata("originaltype") != "":
		$metadata/container/container.text = \
		str(module.get_metadata("container"), " (", module.get_metadata("container_long"), ")")
	else:
		$metadata/container/originaltype.text = ""

	$metadata/container/tracker.text = str(module.get_metadata("tracker"))
	$metadata/container/artist.text = str(module.get_metadata("artist"))
	$metadata/container/title.text = str(module.get_metadata("title"))
	$metadata/container/date.text = str(module.get_metadata("date"))
	$metadata/container/message.text = str(module.get_metadata("message"))
	$metadata/container/warnings.text = str(module.get_metadata("warnings"))

func init_module() -> void:
	if not module.is_module_loaded():
		info.text = "Could not load '" + module.module_file_path +  "'"
		set_process(false)
		return

	labels.clear()

	for i in container.get_children():
		container.remove_child(i)
		i.queue_free()

	$patternscroller.set_size(Vector2(0,0))

	for i in range(module.get_num_channels()):
		var label = Label.new()
		labels.append(label)
		container.add_child(label)

	module.set_audio_generator_playback(generator_player)
	seek.max_value = module.get_duration_seconds()
	set_metadata()
	file_name.text = module.module_file_path
	generator_player.play()

func _ready() -> void:
	spectrum = AudioServer.get_bus_effect_instance(AudioServer.get_bus_index("Master"), 0)
	init_module()

func update_labels() -> void:
	var pattern: int = module.get_current_pattern()
	for i in range(module.get_num_channels()):
		labels[i].text = module.get_channel_string(pattern, i)

func update_label_volumes() -> void:
	for i in range(module.get_num_channels()):
		labels[i].rect_scale.y = 1 + module.get_current_channel_volume(i) / 10.0
		labels[i].modulate = Color.darkgray.darkened(0.8).lightened(module.get_current_channel_volume(i) / 1.2)
	if module.get_current_row() != prev_row:
		prev_row = module.get_current_row()
		tween.interpolate_property(
			patternscroller,
			"rect_position:y",
			patternscroller.rect_position.y,
			get_viewport_rect().size.y/2 - patternscroller.rect_size.y * module.get_current_row() / module.get_pattern_num_rows(module.get_current_pattern()),
			0.1,
			Tween.TRANS_LINEAR,
			Tween.EASE_IN_OUT)
		tween.start()

func _process(delta: float) -> void:
	update_label_volumes()
	info.text = str(
		"speed: ", module.get_current_speed(), "\n",
		"tempo: ", module.get_current_tempo(), "\n",
		"bpm: ", module.get_current_estimated_bpm(), "\n",
		"order: ", module.get_current_order(), "/", module.get_num_orders(), "\n",
		"order_pattern: ", module.get_order_pattern(module.get_current_order()), "\n",
		"row: ", module.get_current_row(), "\n",
		"num_channels: ", module.get_num_channels(), "\n",
		"position: ", module.get_position_seconds(), "/", module.get_duration_seconds(), "\n",
		"playing_channels: ", module.get_current_playing_channels(), "\n"
	)
	volume.value = (60 + linear2db(spectrum.get_magnitude_for_frequency_range(0, 11050).length())) * 100 / 60

	if update_seek:
		seek.value = module.get_position_seconds()
	if prev_pattern != module.get_current_pattern():
		update_labels()
		prev_pattern = module.get_current_pattern()


func _on_HSlider_value_changed(value: float) -> void:
	generator_player.pitch_scale = value

func _on_HSlider2_value_changed(value: float) -> void:
	seek.value = value

func _on_CheckButton_toggled(button_pressed: bool) -> void:
	if button_pressed:
		module.start(false)
	else:
		module.stop()

func _on_HSlider2_drag_started() -> void:
	update_seek = false

func _on_HSlider2_drag_ended(value_changed: bool) -> void:
	update_seek = true
	module.set_position_seconds(seek.value)

func _on_set_seek_gui_input(event: InputEvent) -> void:
	if event == InputEventMouseButton:
		if event.button_index == BUTTON_LEFT:
			module.set_position_seconds(seek.value)


func _on_load_button_pressed() -> void:
	$file.popup_centered(Vector2(400, 300))
	yield($file, "file_selected");
	set_process(false)
	module.stop()
	module.module_file_path = $file.current_path
	if not module.is_module_loaded():
		info.text = "Could not load '" + module.module_file_path +  "'"
	else:
		init_module()
		set_process(true)

func _on_show_controls_toggled(button_pressed: bool) -> void:
	$playback_controls.visible = button_pressed
	$metadata.visible = button_pressed
