libopenmpt-godot
================

This is a GDNative library that allows you to use [libopenmpt][libopenmpt]
within the Godot Engine.

## Related Reading

*See [this article][article] for a detailed description of why this was made.*

## Compilation Instructions

You will have to first download the [Godot C++ Bindings][godot-cpp] for your
target Godot version and then place the `godot-cpp` folder in the project root
folder.

You will need to have libopenmpt (>= 0.6.5) installed in your system to compile
this library.

### For Windows

Download the official compiled binaries from [lib.openmpt.org][libopenmpt-dl]
and extract them into `deps/libopenmpt`.


### For Linux

First you need to compile libopenmpt for your system, if you don't have it
installed already and don't have the appropriate version.

First, obtain the source code from [lib.openmpt.org][libopenmpt-dl] and extract
it into `deps/libopenmpt`, open a terminal in the same directory and run the
following commands:

```
CPPFLAGS=-fPIC ./configure --prefix=$(realpath ./install-files) --disable-openmpt123
make
make install
```

This will put the required libraries into `deps/libopenmpt/install-files`.


## Building

In the root directory, open a terminal and run:

```
scons platform=<platform name> target=<debug/release>
```

to build binaries for your system.


## Advanced Configuration

You can edit the `SConstruct` file in case you are not able to perform the build
properly or want to change libopenmpt's library or header file directories.
These are the lines that you should look for and change:


```
# libopenmpt-specific variables
# The windows version is based on the official libopenmpt distribution
# The linux version is based on direct compilation of the library.
if platform == "windows":
	libopenmpt_path = "deps/libopenmpt"
	libopenmpt_headers_path = libopenmpt_path + "/inc"
	libopenmpt_lib_path = libopenmpt_path + "/lib"
else:
	libopenmpt_path = "deps/libopenmpt/install-files"
	libopenmpt_headers_path = libopenmpt_path + "/include"
	libopenmpt_lib_path = libopenmpt_path + "/lib"

```

## Post-Installation

You will want to set the platform-specific paths for the compiled library in
`project/gdnative/libopenmpt-godot.gdnlib`. Curently only the 64-bit Linux path
is set.

## Usage

See the `project` directory for an example Godot Project. The GDNative class
implements the same interface as `openmpt::module`. The NativeScript must be
first be initialized either by assigning the script to a node or possibly
instantiated and to a script variable using `new()`.

Then the `module_file_path` property of the object must be set to a valid module
file.

```
var module = $path/to/module/node
module.module_file_path = "song.it"
```


An `AudioStreamPlayer` node must be created with the `stream` field/variable
set to an `AudioStreamGenerator`. Then the `buffer_length` field of the
`AudioStreamGenerator` must be set to a small value (say `0.1`) so that the
song and our current song position information do not go out of sync.

Then the `AudioStreamPlayer` must be assigned to the script object
like this:

```
module.set_audio_generator_playback(audio_stream_player)
```

This will now allow the `audio_stream_player` to play the audio being rendered
by the library.

To actually start the rendering, call the start() function:

```
module.start()
```

## Bugs and Issues

Currently `set_audio_generator_playback` only supports an `AudioStreamPlayer`
as an argument. This could be easily resolved by declaring more functions to
accept the other `AudioStreamPlayer` types. The function body should remain
the same, except for the type.

If you find any more bugs and issues with the library, feel free to report them
in the [Github Issues][github-issues] section.


## Licensing

This project is licensed under the MIT/X11 license. See `LICENSE` for details.


[article]: https://visphort.net/articles/3
[libopenmpt]: https://lib.openmpt.org/
[libopenmpt-dl]: https://lib.openmpt.org/libopenmpt/download/
[github-issues]: https://github.com/visphort/libopenmpt-godot/issues
