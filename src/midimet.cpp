/*!
 * @file midimet.cpp
 * @brief Implements the MidiMet MIDI worker class for the Met Module.
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
#include <cmath>
#include <cstdint>
#include <cstdio>
#include "midimet.h"


MidiMet::MidiMet()
{
    vel = 0;
    size = 4;
    res = 1;
    nPoints = res * size;
    channelOut = 0;
    notelength = 180;
    timeshift = 0;
    timeshift_ticks = 0;
    midiNoteKey = 57;
    isMuted = false;
    nPoints = 1;
    framePtr = 0;

    outFrame.resize(2);
    
    midiSample = {0, 0, 0, false};
    midiSample.data = 60;
    midiSample.value = 0;
    
    midiSample.tick = TPQN / res;
    midiSample.muted = false;
    
    outFrame[0] = midiSample;
    midiSample.data = -1;
    outFrame[1] = midiSample;

}

void MidiMet::getNextFrame(int64_t tick)
{
    const int frame_nticks = TPQN / res;
    Sample sample = {0, 0, 0, false};
    if (framePtr == 0) {
        sample.data = midiNoteKey + 12;
    }
    else {
        sample.data = midiNoteKey;
    }
        

    framePtr++;
    framePtr %= res * size;

    if (nextTick < (tick - frame_nticks)) nextTick = tick;

    if (!(framePtr % 2)) {
        /* round-up to current resolution (quantize) */
        nextTick/=frame_nticks;
        nextTick*=frame_nticks;
    }
    
    sample.tick = nextTick;
    outFrame[0] = sample;
    
    nextTick += frame_nticks;
    sample.tick = nextTick;
    outFrame[1] = sample;
}

void MidiMet::updateNoteLength(int val)
{
    notelength = val;
}

void MidiMet::updateTimeShift(int val)
{
    timeshift = val;
    timeshift_ticks = timeshift * TPQN * 1e-3;
}

void MidiMet::updateResolution(int val)
{
    res = val;
    resizeAll();
}

void MidiMet::updateSize(int val)
{
    size = val;
    resizeAll();
}

void MidiMet::resizeAll()
{
    const int npoints = res * size;

    framePtr%=npoints;
    nPoints = npoints;
}

void MidiMet::updateVelocity(int val)
{
    vel = val;
}

void MidiMet::setFramePtr(int ix)
{
    framePtr=ix;
}

void MidiMet::setNextTick(uint64_t tick)
{
    int pos = (tick * res / TPQN) % nPoints;

    setFramePtr(pos);
    nextTick = tick;
}

void MidiMet::setMuted(bool on)
{
    isMuted = on;
}
