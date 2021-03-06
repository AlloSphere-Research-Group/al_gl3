#ifndef INCLUDE_AL_AUDIO_IO_HPP
#define INCLUDE_AL_AUDIO_IO_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012. The Regents of the University of California. All
   rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are
   met:

                Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

                Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the
   distribution.

                Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
                this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        File description:
        An interface to low-level audio device streams

        File author(s):
        Lance Putnam, 2010, putnam.lance@gmail.com
        Andres Cabrera, 2017 mantaraya36@gmail.com
*/

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "al/io/al_AudioIOData.hpp"

namespace al {

/// Audio callback type
typedef void (*audioCallback)(AudioIOData &io);

class AudioDevice;

/// Abstract audio backend
///
/// @ingroup IO
class AudioBackend {
public:
  AudioBackend();

  ~AudioBackend() {}

  bool isOpen() const;
  bool isRunning() const;
  bool error() const;

  void printError(const char *text = "") const;
  void printInfo() const;

  bool supportsFPS(double fps);

  void inDevice(int index);
  void outDevice(int index);

  void channels(int num, bool forOutput);
  void setStreamName(std::string name);

  int inDeviceChans();
  int outDeviceChans();
  void setInDeviceChans(int num);
  void setOutDeviceChans(int num);

  double time();

  bool open(int framesPerSecond, unsigned int framesPerBuffer, void *userdata);
  bool close();

  bool start(int framesPerSecond, int framesPerBuffer, void *userdata);
  bool stop();
  double cpu();

  // Device information
  static AudioDevice defaultInput();
  static AudioDevice defaultOutput();
  static bool deviceIsValid(int num);
  static int deviceMaxInputChannels(int num);
  static int deviceMaxOutputChannels(int num);
  static double devicePreferredSamplingRate(int num);
  static std::string deviceName(int num);
  static int numDevices();

protected:
  bool mRunning{false};
  bool mOpen{false};
  std::shared_ptr<void> mBackendData;
};

/// Audio device
///
/// @ingroup IO
class AudioDevice : public AudioDeviceInfo {
public:
  /// Stream mode
  enum StreamMode {
    INPUT = 1, /**< Input stream */
    OUTPUT = 2 /**< Output stream */
  };

  /// @param[in] deviceNum	Device enumeration number
  AudioDevice(int deviceNum = -1);

  /// @param[in] nameKeyword	Keyword to search for in device name
  /// @param[in] stream		Whether to search for input and/or output
  /// devices
  AudioDevice(const std::string &nameKeyword,
              StreamMode stream = StreamMode(INPUT | OUTPUT));

  virtual bool valid() const { return mValid; }
  virtual bool hasInput() const {
    return channelsInMax() > 0;
  } ///< Returns whether device has input
  virtual bool hasOutput() const {
    return channelsOutMax() > 0;
  } ///< Returns whether device has output

  virtual void
  print() const; ///< Prints info about specific i/o device to stdout

  static AudioDevice defaultInput();  ///< Get system's default input device
  static AudioDevice defaultOutput(); ///< Get system's default output device
  static int numDevices(); ///< Returns number of audio i/o devices available
  static void
  printAll(); ///< Prints info about all available i/o devices to stdout

protected:
  void setImpl(int deviceNum);
  static void initDevices();
};

inline AudioDevice::StreamMode operator|(const AudioDevice::StreamMode &a,
                                         const AudioDevice::StreamMode &b) {
  return static_cast<AudioDevice::StreamMode>(+a | +b);
}

/// Audio input/output streaming
///
/// @ingroup IO
class AudioIO : public AudioIOData {
public:
  /// Creates AudioIO using default I/O devices.
  AudioIO();
  virtual ~AudioIO();
  /// @param[in] framesPerBuf	Number of sample frames to process per callback
  /// @param[in] framesPerSec	Frame rate.  Unsupported values will use default
  /// rate of device.
  /// @param[in] outChans		Number of output channels to open
  /// @param[in] inChans		Number of input channels to open
  /// If the number of input or output channels is greater than the device
  /// supports, virtual buffers will be created.
  void init(void (*callback)(AudioIOData &), void *userData,
            int framesPerBuf = 64, double framesPerSec = 44100.0,
            int outChans = 2, int inChans = 0);

  void init(void (*callback)(AudioIOData &), void *userData, AudioDevice &dev,
            int framesPerBuf = 64, double framesPerSec = 44100.0,
            int outChans = 2, int inChans = 0);

  /// @param[in] callback	Audio processing callback
  /// (optional)
  /// @param[in] userData	Pointer to user data accessible within callback
  /// (optional)
  /// @param[in] use_in		Enable audio input
  /// @param[in] use_out	Enable audio output
  /// @param[in] devNum		ID of the device to open. -1 Uses default
  /// device.
  /// @param[in] framesPerBuf	Number of sample frames to process per callback
  void initWithDefaults(void (*callback)(AudioIOData &), void *userData,
                        bool use_out, bool use_in, int framesPerBuffer = 256);

  bool open();  ///< Opens audio device.
  bool close(); ///< Closes audio device. Will stop active IO.
  bool start(); ///< Starts the audio IO.  Will open audio device if necessary.
  bool stop();  ///< Stops the audio IO.
  void processAudio(); ///< Call callback manually

  bool isOpen();    ///< Returns true if device has been opened
  bool isRunning(); ///< Returns true if audio is running

  bool autoZeroOut() const { return mAutoZeroOut; }
  int channelsInDevice()
      const; ///< Get number of channels opened on input device
  int channelsOutDevice()
      const; ///< Get number of channels opened on output device
  bool clipOut() const { return mClipOut; } ///< Returns clipOut setting
  double cpu() const; ///< Returns current CPU usage of audio thread
  bool
  supportsFPS(double fps); ///< Return true if fps supported, otherwise false
  bool zeroNANs()
      const; ///< Returns whether to zero NANs in output buffer going to DAC

  /// Sets number of effective channels on input or output device depending on
  /// 'forOutput' flag.
  /// An effective channel is either a real device channel or virtual channel
  /// depending on how many channels the device supports. Passing in -1 for
  /// the number of channels opens all available channels.
  void channels(int num, bool forOutput) override;
  void channelsBus(int num) override; ///< Set number of bus channels

  /// Set name of this stream. Currently only has an effect when using jack
  void setStreamName(std::string name);

  void clipOut(bool v); ///< Set whether to clip output between -1 and 1
  void
  device(const AudioDevice &v); ///< Set input/output device (must be duplex)
  void deviceIn(const AudioDevice &v);  ///< Set input device
  void deviceOut(const AudioDevice &v); ///< Set output device
  virtual void
  framesPerSecond(double v) override; ///< Set number of frames per second
  virtual void framesPerBuffer(
      unsigned int n) override; ///< Set number of frames per processing buffer
  void zeroNANs(bool v) {
    mZeroNANs = v;
  } ///< Set whether to zero NANs in output buffer going to DAC

  void print() const; ///< Prints info about current i/o devices to stdout.
  static const char *errorText(int errNum); ///< Returns error string.

  double time() const;          ///< Get current stream time in seconds
  double time(int frame) const; ///< Get current stream time in seconds of frame

  /// Add an AudioCallback handler (internal callback is always called first)
  AudioIO &append(AudioCallback &v);
  AudioIO &prepend(AudioCallback &v);
  AudioIO &insertBefore(AudioCallback &v, AudioCallback &beforeThis);
  AudioIO &insertAfter(AudioCallback &v, AudioCallback &afterThis);

  /// Remove all input event handlers matching argument
  AudioIO &remove(AudioCallback &v);

  using AudioIOData::channelsBus;
  using AudioIOData::channelsIn;
  using AudioIOData::channelsOut;
  using AudioIOData::framesPerBuffer;
  using AudioIOData::framesPerSecond;

  audioCallback callback; ///< User specified callback function.

private:
  AudioDevice mInDevice, mOutDevice;
  bool mZeroNANs;    // whether to zero NANs
  bool mClipOut;     // whether to clip output between -1 and 1
  bool mAutoZeroOut; // whether to automatically zero output buffers each block
  std::vector<AudioCallback *> mAudioCallbacks;

  void reopen(); // reopen stream (restarts stream if needed)
  void resizeBuffer(bool forOutput);
  void operator=(const AudioIO &) = delete; // Disallow copy

  std::unique_ptr<AudioBackend> mBackend;
};

} // namespace al

#endif
