/*!
 * @file midimet_lv2.h
 * @brief Headers for the MidiMet LV2 plugin class
 *
 *
 *      Copyright 2009 - 2026 <qmidiarp-devel@lists.sourceforge.net>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *
 */

#ifndef MIDIMET_LV2_H
#define MIDIMET_LV2_H

#include "midimet.h"

#define MIDIMET_LV2_URI "https://github.com/emuse/midimet"

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"


typedef struct {
    LV2_URID atom_Object;
    LV2_URID atom_Blank;
    LV2_URID atom_Float;
    LV2_URID atom_Int;
    LV2_URID atom_Vector;
    LV2_URID atom_Long;
    LV2_URID atom_String;
    LV2_URID atom_eventTransfer;
    LV2_URID atom_Resource;
    LV2_URID time_Position;
    LV2_URID time_frame;
    LV2_URID time_barBeat;
    LV2_URID time_beatsPerMinute;
    LV2_URID time_speed;
    LV2_URID midi_MidiEvent;
    LV2_URID atom_Sequence;
} MidiMetURIs;

static inline void map_uris(LV2_URID_Map* urid_map, MidiMetURIs* uris) {
    uris->atom_Object         = urid_map->map(urid_map->handle, LV2_ATOM__Object);
    uris->atom_Blank          = urid_map->map(urid_map->handle, LV2_ATOM__Blank);
    uris->atom_Float          = urid_map->map(urid_map->handle, LV2_ATOM__Float);
    uris->atom_Int            = urid_map->map(urid_map->handle, LV2_ATOM__Int);
    uris->atom_Vector         = urid_map->map(urid_map->handle, LV2_ATOM__Vector);
    uris->atom_Long           = urid_map->map(urid_map->handle, LV2_ATOM__Long);
    uris->atom_String         = urid_map->map(urid_map->handle, LV2_ATOM__String);
    uris->atom_eventTransfer  = urid_map->map(urid_map->handle, LV2_ATOM__eventTransfer);
    uris->atom_Resource       = urid_map->map(urid_map->handle, LV2_ATOM__Resource);
    uris->time_Position       = urid_map->map(urid_map->handle, LV2_TIME__Position);
    uris->time_frame          = urid_map->map(urid_map->handle, LV2_TIME__frame);
    uris->time_barBeat        = urid_map->map(urid_map->handle, LV2_TIME__barBeat);
    uris->time_beatsPerMinute = urid_map->map(urid_map->handle, LV2_TIME__beatsPerMinute);
    uris->time_speed          = urid_map->map(urid_map->handle, LV2_TIME__speed);
    uris->midi_MidiEvent      = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);
    uris->atom_Sequence       = urid_map->map(urid_map->handle, LV2_ATOM__Sequence);
}


class MidiMetLV2 : public MidiMet
{
public:

        MidiMetLV2(double sample_rate, const LV2_Feature *const *host_features);

        ~MidiMetLV2();
        /* this enum is for the float value array and shifted by 2 compared with the
         * float port indices */
        enum FloatField {
            VELOCITY = 0,
            NOTELENGTH = 1,
            RESOLUTION = 2,
            SIZE = 3,
            CH_OUT = 4,
            CURSOR_POS = 5, //output
            MUTE = 6,
            TRANSPORT_MODE = 7,
            TEMPO_MODE = 8,
            TEMPO = 9,
            HOST_TEMPO = 10,
            HOST_POSITION = 11,
            HOST_SPEED = 12,
            TIMESHIFT = 13
        };
        enum State {
          STATE_ATTACK, // Envelope rising
          STATE_DECAY,  // Envelope lowering
          STATE_OFF     // Silent
        };
    
        void connect_port(uint32_t port, void *data);
        void run(uint32_t nframes);
        void activate();
        void deactivate();
        void updatePosAtom(const LV2_Atom_Object* obj);
        void updatePos(uint64_t position, float bpm, int speed, bool ignore_pos=false);
        void initTransport();
        LV2_URID_Map *uridMap;
        MidiMetURIs m_uris;
        LV2_Atom_Forge forge;
        LV2_Atom_Forge_Frame m_frame;

private:

        float *outputPort;
        float *val[17];

        // One cycle of a sine wave
        float*   wave_h;
        float*   wave_l;
        float*   clock_fm;
        uint32_t wave_len;        
        
        uint64_t curFrame;
        uint64_t soundOnFrame;
        uint64_t tempoChangeTick;
        uint64_t curTick;
        uint32_t elapsed_len; // Frames since the start of the last click

        Sample currentSample;

        double internalTempo;
        double sampleRate;
        double tempo;
        bool transportAtomReceived;

        void updateParams();
        void forgeMidiEvent(uint32_t f, const uint8_t* const buffer, uint32_t size);

        uint64_t transportFramesDelta;  /**< Frames since last click start */
        float transportBpm;
        float transportSpeed;
        bool hostTransport;
        bool tempoFromHost; /**< 0: Internal, 1: Host */
        uint32_t evQueue[JQ_BUFSZ];
        uint64_t evTickQueue[JQ_BUFSZ];
        int bufPtr;

        LV2_Atom_Sequence *inEventBuffer;
        const LV2_Atom_Sequence *outEventBuffer;

        int sliderToTickLen(int val) { return (val * TPQN / 64); }
};

#endif
