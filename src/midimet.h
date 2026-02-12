/*!
 * @file midimet.h
 * @brief Member definitions for the MidiMet MIDI worker class.
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

#ifndef MIDIMET_H
#define MIDIMET_H

#include <cstdint>
#include <vector>
#include "midievent.h"

#define TPQN           48000
#define JQ_BUFSZ        1024

/*! @brief This array holds the currently available Seq resolution values.
 */
const int seqResValues[13] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 16};
/*! @brief This array holds the currently available Seq size values.
 */
const int seqSizeValues[20] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 24, 32, 64, 128};


/*! @brief MIDI worker class for the Met Module. Implements a metronome
 * connectable to transport.
 *
 * The parameters of MidiMet are controlled by the MetWidget class.
 * The backend driver thread calls the Engine::echoCallback(), which will
 * query each module, in this case via
 * the MidiMet::getNextFrame() method. MidiMet will produce a click at
 * configurable beat divisions
 */
class MidiMet  {

  public:
    int vel, notelength;
    int timeshift;
    int res, size;
    int channelOut;
    int midiNoteKey;
    bool isMuted;
    int64_t nextTick; /*!< Holds the next tick at which note events will be played out */
    int framePtr;       /*!< position of the currently output frame in sequence/wave/pattern */
    int nPoints;        /*!< Number of steps in pattern or sequence or wave */
    Sample midiSample;
    
    std::vector<Sample> outFrame;   /*!< Vector of Sample points holding the current frame for transfer */

  public:
    MidiMet();
    virtual ~MidiMet() {}
    
    void updateVelocity(int);
    void updateResolution(int);
    void updateNoteLength(int);
    void updateTimeShift(int);
    void updateSize(int);

    void resizeAll();

/*! @brief sets MidiWorker::isMuted, which is checked by
 * Engine and which suppresses data output globally if set to True.
 *
 * @param on Set to True to suppress data output to the Driver
 */
    virtual void setMuted(bool on);
    
    void setNextTick(uint64_t tick);
/*! @brief  transfers the next Midi data Frame to an intermediate internal object
 * 
 * @param tick the current tick at which we request a note. This tick will be
 * used to calculate the nextTick which is quantized to the pattern
 */
    void getNextFrame(int64_t tick);
    void setFramePtr(int ix);
    int getFramePtr() { return framePtr; }
};

#endif
