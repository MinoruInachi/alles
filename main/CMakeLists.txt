set(AMY_SRC_DIR ../amy/src)
set(ALLES_SRCS
	alles.c
	alles_esp32.c
	multicast_esp32.c
	buttons.c
	sounds.c
	${AMY_SRC_DIR}/algorithms.c
	${AMY_SRC_DIR}/amy.c
	${AMY_SRC_DIR}/amy_midi.c
	${AMY_SRC_DIR}/api.c
	${AMY_SRC_DIR}/custom.c
	${AMY_SRC_DIR}/cv_trigger.c
	${AMY_SRC_DIR}/delay.c
	${AMY_SRC_DIR}/envelope.c
	${AMY_SRC_DIR}/filters.c
	${AMY_SRC_DIR}/i2s.c
	${AMY_SRC_DIR}/instrument.c
	${AMY_SRC_DIR}/interp_partials.c
	${AMY_SRC_DIR}/log2_exp2.c
	${AMY_SRC_DIR}/midi_mappings.c
	${AMY_SRC_DIR}/note_output.c
	${AMY_SRC_DIR}/oscillators.c
	${AMY_SRC_DIR}/parse.c
	${AMY_SRC_DIR}/patches.c
	${AMY_SRC_DIR}/pcm.c
	${AMY_SRC_DIR}/sequencer.c
	${AMY_SRC_DIR}/transfer.c
)

if(IDF_TARGET STREQUAL "esp32s3")
	# M5Stack Atom VoiceS3R
	list(APPEND ALLES_SRCS codec_es8311.c atom_battery.c)
elseif(CONFIG_ALLES_ATOM_VOICE)
	# M5Stack Atom Voice
	list(APPEND ALLES_SRCS ns4168.c)
else()
	list(APPEND ALLES_SRCS power.c)
endif()

idf_component_register(SRCS ${ALLES_SRCS}
                    INCLUDE_DIRS ${AMY_SRC_DIR} .)

target_compile_options(${COMPONENT_LIB} PRIVATE
	-DALLES
	-Wno-uninitialized
)

if(CONFIG_ALLES_ATOM_VOICE)
	# No PSRAM, so size AMY for internal RAM: its task stacks (measured peaks ~1KB render, ~2.5KB fill),
	# the delta pool (grows in blocks on demand) and the MIDI sysex buffer (no MIDI on the Atom)
	target_compile_definitions(${COMPONENT_LIB} PRIVATE
		AMY_RENDER_TASK_STACK_SIZE=4096
		AMY_FILL_BUFFER_TASK_STACK_SIZE=6144
		DELTA_BLOCK_SIZE=512
		MAX_SYSEX_BYTES=256
	)
endif()

set_source_files_properties(alles_esp32.c alles.c
    PROPERTIES COMPILE_FLAGS
    -Wno-strict-aliasing
)

# AMY's show_debug() calls our esp_show_debug() under ALLES without a prototype
set_source_files_properties(${AMY_SRC_DIR}/amy.c
    PROPERTIES COMPILE_FLAGS
    "-Wno-strict-aliasing -Wno-error=implicit-function-declaration -Wno-implicit-function-declaration"
)
