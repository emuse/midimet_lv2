/*!
 * @file midimet_lv2.cpp
 * @brief Implements an LV2 plugin inheriting from MidiMet
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

#include <cstdio>
#include <cmath>
#include "midimet_lv2.h"

MidiMetLV2::MidiMetLV2 (
    double sample_rate, const LV2_Feature *const *host_features )
    :MidiMet()
{
    for (int l1 = 0; l1 < 17; l1++) val[l1] = 0;

    sampleRate = sample_rate;
    curFrame = 0;
    soundOnFrame = 0;
    inEventBuffer = NULL;
    outEventBuffer = NULL;
    tempo = 120.0f;
    internalTempo = 120.0f;

    transportBpm = 120.0f;
    transportFramesDelta = 0;
    curTick = 0;
    
    tempoChangeTick = 0;
    hostTransport = false;
    transportSpeed = 1;
    transportAtomReceived = false;
    
    for (int l1 = 0; l1 < JQ_BUFSZ; l1++) {
        evQueue[l1] = 0;
        evTickQueue[l1] = 0;
    }


    bufPtr = 0;
    elapsed_len = 0;

    LV2_URID_Map *urid_map;

    //Generate "FM" wave
    const float amp = .5;
    
    // Carrier (position 0) and Modulator Oscillator parameters
    const float FH[5] = {880,       1.5,    4,    8,    12}; // Hz
    const float FL[5] = {440,       1.5,    4,    8,    12}; // Hz
    const int   A[5] =  {1,   12,      8,    8,    10};
    // const int   A[5] =  {1,   8,      8,    5,    10};
    // const int   A[5] =  {1,   0,      0,    0,    0};
    const float T[5] =  {.02,     .015,   .01,  .01,  .005}; // seconds
    
    // We are cutting of the wave when time is beyond 30 decay times
    const uint32_t npoints = (int)(30 * T[0] * sampleRate);
        
    wave_len = npoints;
    
    clock_fm          = (float*)malloc(npoints * sizeof(float));
    wave_h            = (float*)malloc(npoints * sizeof(float));
    wave_l            = (float*)malloc(npoints * sizeof(float));

    for (uint32_t clock = 0; clock < npoints; clock++) {
        clock_fm[clock] = clock;
    }
    for (uint32_t i = 1; i < 5; i++) {
        for (uint32_t clock = 0; clock < npoints; clock++) {
            clock_fm[clock] += (sin((FH[i] * M_PI * FH[0] / sampleRate) * clock) * A[i]) *
                                exp(-1.* clock / npoints /T[i]);
        }
    }
    
    for (uint32_t clock = 0; clock < npoints; clock++) {
        wave_h[clock] = (sin((2 * M_PI * FH[0] / sampleRate) * clock_fm[clock]) * A[0])
                        * exp(-1. * clock / npoints /T[0])
                        * amp;
        wave_l[clock] = (sin((2 * M_PI * FL[0] / sampleRate) * clock_fm[clock]) * A[0])
                        * exp(-1. * clock / npoints /T[0])
                        * amp * 2;
    }


    /* Scan host features for URID map */

    for (int i = 0; host_features[i]; ++i) {
        if (::strcmp(host_features[i]->URI, LV2_URID_URI "#map") == 0) {
            urid_map = (LV2_URID_Map *) host_features[i]->data;
            }
    }
    if (!urid_map) {
        printf("Host does not support urid:map.\n");
        return;
    }

    lv2_atom_forge_init(&forge, urid_map);

    /* Map URIS */
    MidiMetURIs* const uris = &m_uris;
    map_uris(urid_map, uris);
    uridMap = urid_map;
}


MidiMetLV2::~MidiMetLV2 (void)
{
}

void MidiMetLV2::connect_port ( uint32_t port, void *seqdata )
{
    switch(port) {
    case 0:
        outputPort = (float*)seqdata;
        break;
    case 1:
        outEventBuffer = (const LV2_Atom_Sequence*)seqdata;
        break;
    case 2:
        inEventBuffer = (LV2_Atom_Sequence*)seqdata;
        break;
    default:
        val[port - 3] = (float *)seqdata;
        break;
    }
}

void MidiMetLV2::updatePosAtom(const LV2_Atom_Object* obj)
{
    MidiMetURIs* const uris = &m_uris;

    uint64_t pos1 = transportFramesDelta;
    float bpm1 = tempo;
    int speed1 = transportSpeed;

    // flag that the host sends transport information via atom port and
    // that we will no longer process designated port events
    transportAtomReceived = true;

    LV2_Atom *bpm = NULL, *speed = NULL, *pos = NULL;
    lv2_atom_object_get(obj,
                        uris->time_frame, &pos,
                        uris->time_beatsPerMinute, &bpm,
                        uris->time_speed, &speed,
                        NULL);

    if (bpm && bpm->type == uris->atom_Float) bpm1 = ((LV2_Atom_Float*)bpm)->body;
    if (pos && pos->type == uris->atom_Long)  pos1 = ((LV2_Atom_Long*)pos)->body;
    if (speed && speed->type == uris->atom_Float) speed1 = ((LV2_Atom_Float*)speed)->body;

    updatePos(pos1, bpm1, speed1);
}

void MidiMetLV2::updatePos(uint64_t pos, float bpm, int speed, bool ignore_pos)
{
    
    transportBpm = bpm;

    if (hostTransport || tempoFromHost) {
        if (tempo != bpm) {
            /* Tempo changed */
            tempo = transportBpm;
            if (hostTransport) {
                transportSpeed = 0;
            }
            initTransport();
        }
    }
    if (hostTransport) {
        if (!ignore_pos) {
            const float frames_per_beat = 60.0f / transportBpm * sampleRate;
            transportFramesDelta = pos;
            tempoChangeTick = pos * TPQN / frames_per_beat;
        }    
        if (transportSpeed != speed) {
            /* Speed changed, e.g. 0 (stop) to 1 (play) */
            transportSpeed = speed;
            curFrame = transportFramesDelta;
            if (transportSpeed) {
                setNextTick(tempoChangeTick);
            }     
        }
    }
}

void MidiMetLV2::run (uint32_t nframes )
{
    const int32_t timeshift_ticks = timeshift * TPQN * tempo / 60. * 1e-3;
    float* const   output          = outputPort;
    const MidiMetURIs* uris = &m_uris;
    const uint32_t capacity = outEventBuffer->atom.size;
    lv2_atom_forge_set_buffer(&forge, (uint8_t*)outEventBuffer, capacity);
    lv2_atom_forge_sequence_head(&forge, &m_frame, 0);

    updateParams();

    if (inEventBuffer) {
        LV2_ATOM_SEQUENCE_FOREACH(inEventBuffer, event) {
            // Control Atom Input
            if (event && (event->body.type == uris->atom_Object
                        || event->body.type == uris->atom_Blank)) {
                const LV2_Atom_Object* obj = (LV2_Atom_Object*)&event->body;
                if (obj->body.otype == uris->time_Position) {
                    /* Received position information, update */
                    updatePosAtom(obj);
                }
            }
        }
    }

        // MIDI Output
    for (uint32_t f = 0 ; f < nframes; f++) {
        curTick = (uint64_t)(curFrame - transportFramesDelta)
                        *TPQN*tempo/60/sampleRate + tempoChangeTick;
        if (timeshift_ticks > 0) {
            if (curTick > (uint32_t)(timeshift_ticks)) {
                curTick -= timeshift_ticks;
            }
        }
        else {
            curTick -= timeshift_ticks;
        }
        if ((curTick >= (uint64_t)nextTick) && (transportSpeed)) {
            getNextFrame(nextTick);
            if (!outFrame[0].muted && !isMuted) {
                unsigned char d[3];
                d[0] = 0x90 + channelOut;
                d[1] = outFrame[0].data;
                d[2] = vel;
                forgeMidiEvent(f, d, 3);
                soundOnFrame = curFrame;
                evTickQueue[bufPtr] = curTick + notelength / 4;
                evQueue[bufPtr] = outFrame[0].data;
                bufPtr++;
            }
            float pos = (float)getFramePtr();
            *val[CURSOR_POS] = pos;
        }
        // Note Off Queue handling
        uint64_t noteofftick = evTickQueue[0];
        int idx = 0;
        for (int l1 = 0; l1 < bufPtr; l1++) {
            uint64_t tmptick = evTickQueue[l1];
            if (noteofftick > tmptick) {
                idx = l1;
                noteofftick = tmptick;
            }
        }
        if ( (bufPtr) && ((curTick >= noteofftick)
                || (hostTransport && !transportSpeed)) ) {
            int outval = evQueue[idx];
            for (int l4 = idx ; l4 < (bufPtr - 1);l4++) {
                evQueue[l4] = evQueue[l4 + 1];
                evTickQueue[l4] = evTickQueue[l4 + 1];
            }
            bufPtr--;

            unsigned char d[3];
            d[0] = 0x80 + channelOut;
            d[1] = outval;
            d[2] = 127;
            forgeMidiEvent(f, d, 3);
        }
        elapsed_len  = curFrame - soundOnFrame;
        if (elapsed_len < wave_len) {
            if (framePtr == 1) {
                output[f] = wave_h[elapsed_len] * vel / 128;
                }
            else {
                output[f] = wave_l[elapsed_len] * vel / 128;
            }
        }
        else {
            output[f] = 0.0f;
        }
        curFrame++;
    }
}

void MidiMetLV2::forgeMidiEvent(uint32_t f, const uint8_t* const buffer, uint32_t size)
{
    MidiMetURIs* const uris = &m_uris;
    LV2_Atom midiatom;
    midiatom.type = uris->midi_MidiEvent;
    midiatom.size = size;
    lv2_atom_forge_frame_time(&forge, f);
    lv2_atom_forge_raw(&forge, &midiatom, sizeof(LV2_Atom));
    lv2_atom_forge_raw(&forge, buffer, size);
    lv2_atom_forge_pad(&forge, sizeof(LV2_Atom) + size);
}

void MidiMetLV2::updateParams()
{
    if (vel != *val[VELOCITY]) {
        vel = *val[VELOCITY];
        updateVelocity(vel);
    }

    if (notelength != sliderToTickLen(*val[NOTELENGTH])) {
        updateNoteLength(sliderToTickLen(*val[NOTELENGTH]));
    }

    if (timeshift != (int)*val[TIMESHIFT]) {
        updateTimeShift((int)*val[TIMESHIFT]);
    }


    if (res != seqResValues[(int)*val[RESOLUTION]]) {
        updateResolution(seqResValues[(int)*val[RESOLUTION]]);
    }

    if (size != seqSizeValues[(int)*val[SIZE]]) {
        updateSize(seqSizeValues[(int)*val[SIZE]]);
    }

    if (isMuted != (bool)*val[MUTE]) setMuted((bool)(*val[MUTE]));

    if (internalTempo != *val[TEMPO]) {
        internalTempo = *val[TEMPO];
        if (!hostTransport) {
            initTransport();
        }
    }

    if (tempoFromHost != (bool)(*val[TEMPO_MODE])) {
        tempoFromHost = (bool)(*val[TEMPO_MODE]);
        initTransport();
    }

    if (hostTransport != (bool)(*val[TRANSPORT_MODE])) {
        hostTransport = (bool)(*val[TRANSPORT_MODE]);
        if (hostTransport) {
            tempoFromHost = true;
        }
         initTransport();
    }

    if (hostTransport && !transportAtomReceived) {
        updatePos(  (uint64_t)*val[HOST_POSITION],
                    (float)*val[HOST_TEMPO],
                    (int)*val[HOST_SPEED],
                    false);
    }

}

void MidiMetLV2::initTransport()
{
    if (tempoFromHost) {
        tempo = transportBpm;
    }
    else {
        tempo = internalTempo;
    }
    
    if (!hostTransport) {
        transportFramesDelta = curFrame;
        if (curTick > 0) tempoChangeTick = curTick;
        transportSpeed = 1;
    }
    else {
        transportSpeed = 0;
        setNextTick(tempoChangeTick);
    }
}

void MidiMetLV2::activate (void)
{
    initTransport();
}

void MidiMetLV2::deactivate (void)
{
    transportSpeed = 0;
}

static LV2_Handle MidiMetLV2_instantiate (
    const LV2_Descriptor *, double sample_rate, const char *,
    const LV2_Feature *const *host_features )
{
    return new MidiMetLV2(sample_rate, host_features);
}

static void MidiMetLV2_connect_port (
    LV2_Handle instance, uint32_t port, void *data )
{
    MidiMetLV2 *pPlugin = static_cast<MidiMetLV2 *> (instance);
    if (pPlugin)
        pPlugin->connect_port(port, data);
}

static void MidiMetLV2_run ( LV2_Handle instance, uint32_t nframes )
{
    MidiMetLV2 *pPlugin = static_cast<MidiMetLV2 *> (instance);
    if (pPlugin)
        pPlugin->run(nframes);
}

static void MidiMetLV2_activate ( LV2_Handle instance )
{
    MidiMetLV2 *pPlugin = static_cast<MidiMetLV2 *> (instance);
    if (pPlugin)
        pPlugin->activate();
}

static void MidiMetLV2_deactivate ( LV2_Handle instance )
{
    MidiMetLV2 *pPlugin = static_cast<MidiMetLV2 *> (instance);
    if (pPlugin)
        pPlugin->deactivate();
}

static void MidiMetLV2_cleanup ( LV2_Handle instance )
{
    MidiMetLV2 *pPlugin = static_cast<MidiMetLV2 *> (instance);
    if (pPlugin)
        delete pPlugin;
}

static const void *MidiMetLV2_extension_data ( const char * uri)
{
    (void)uri;
    return NULL;
}

static const LV2_Descriptor MidiMetLV2_descriptor =
{
    MIDIMET_LV2_URI,
    MidiMetLV2_instantiate,
    MidiMetLV2_connect_port,
    MidiMetLV2_activate,
    MidiMetLV2_run,
    MidiMetLV2_deactivate,
    MidiMetLV2_cleanup,
    MidiMetLV2_extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
    return (index == 0 ? &MidiMetLV2_descriptor : NULL);
}

