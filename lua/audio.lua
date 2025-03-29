local ffi = require("ffi")
local lib = require("micro/lua/libmicro")

ffi.cdef([[
    typedef unsigned int uint32_t;

    int microAudioInit();
    int microAudioDestroy();

    int microSoundLoadFromFile(const char *filepath);
    void microSoundFree(const uint32_t soundId);

    void microSoundPlay(const uint32_t soundId, const float gain);
    void microSoundPlayAt(const uint32_t soundId, const float gain, const float x, const float y);
    void microSoundPlayAtSource(const uint32_t soundId, const uint32_t sourceId, const char *channelTag);

    uint32_t microSoundSrcNew(const char *mainChannelTag);
    uint32_t microSoundSrcNewChannel(const uint32_t sourceId, const char *channelTag);
    void microSoundSrcFree(const uint32_t sourceId);
    int microSoundSrcIsPlaying(const uint32_t sourceId, const char *channelTag);
    void microSoundSrcLoop(const uint32_t sourceId, const char *channelTag, const bool loops);
    void microSoundSrcStop(const uint32_t sourceId, const char *channelTag);
    void microSoundSrcStopAll();
    void microSoundSrcSetPosition(const uint32_t sourceId, float x, float y);
    void microSoundSrcSpatialize(const uint32_t sourceId, const bool spatialize);
    void microSoundSrcPause(const uint32_t sourceId, const char *channelTag);
    void microSoundSrcResume(const uint32_t sourceId, const char *channelTag);
    void microSoundSrcSetVolume(const uint32_t sourceId, const char *channelTag, const float volume);
    float microSoundSrcGetVolume(const uint32_t sourceId, const char *channelTag);

    void microListenerSetPosition(float x, float y);
    void microListenerSetMaxDistance(float distance);
]])

local Audio = {}

function Audio.init()
  local ret = lib.microAudioInit()
  if ret ~= 0 then
    error("Audio initialization failed")
  end
  return ret
end

function Audio.destroy()
  local ret = lib.microAudioDestroy()
  if ret ~= 0 then
    error("Audio destruction failed")
  end
  return ret
end

---@param filepath string
---@return number
function Audio.loadSound(filepath)
  local soundId = lib.microSoundLoadFromFile(filepath)
  if soundId == -1 then
    error("Failed to load sound: " .. filepath)
  end
  return soundId
end

---@param soundId number
function Audio.freeSound(soundId)
  lib.microSoundFree(soundId)
end

---@param soundId number
---@param gain number
function Audio.playSound(soundId, gain)
  gain = gain or 1.0
  lib.microSoundPlay(soundId, gain)
end

---@param soundId number
---@param gain number
---@param x number
---@param y number
function Audio.playSoundAt(soundId, gain, x, y)
  gain = gain or 1.0
  lib.microSoundPlayAt(soundId, gain, x, y)
end

---@param soundId number
---@param sourceId number
---@param channelTag string
function Audio.playSoundAtSource(soundId, sourceId, channelTag)
  lib.microSoundPlayAtSource(soundId, sourceId, channelTag)
end

---@param mainChannelTag string
---@return number
function Audio.newSource(mainChannelTag)
  return lib.microSoundSrcNew(mainChannelTag)
end

---@param sourceId number
---@param channelTag string
function Audio.newChannel(sourceId, channelTag)
  return lib.microSoundSrcNewChannel(sourceId, channelTag)
end

---@param sourceId number
function Audio.freeSource(sourceId)
  lib.microSoundSrcFree(sourceId)
end

---@param sourceId number
---@param channelTag string
---@return boolean
function Audio.isPlaying(sourceId, channelTag)
  return lib.microSoundSrcIsPlaying(sourceId, channelTag) == 1
end

---@param sourceId number
---@param channelTag string
---@param loops boolean
function Audio.setLoop(sourceId, channelTag, loops)
  lib.microSoundSrcLoop(sourceId, channelTag, loops and true or false)
end

---@param sourceId number
---@param channelTag string
function Audio.stop(sourceId, channelTag)
  lib.microSoundSrcStop(sourceId, channelTag)
end

function Audio.stopAll()
  lib.microSoundSrcStopAll()
end

---@param sourceId number
---@param x number
---@param y number
function Audio.setSourcePosition(sourceId, x, y)
  lib.microSoundSrcSetPosition(sourceId, x, y)
end

function Audio.spatialize(sourceId, spatialize)
  lib.microSoundSrcSpatialize(sourceId, spatialize and true or false)
end

---@param sourceId number
---@param channelTag string
function Audio.pause(sourceId, channelTag)
  lib.microSoundSrcPause(sourceId, channelTag)
end

---@param sourceId number
---@param channelTag string
function Audio.resume(sourceId, channelTag)
  lib.microSoundSrcResume(sourceId, channelTag)
end

---@param sourceId number
---@param channelTag string
---@param volume number
function Audio.setVolume(sourceId, channelTag, volume)
  lib.microSoundSrcSetVolume(sourceId, channelTag, volume)
end

---@param sourceId number
---@param channelTag string
---@return number
function Audio.getVolume(sourceId, channelTag)
  return lib.microSoundSrcGetVolume(sourceId, channelTag)
end

---@param x number
---@param y number
function Audio.setListenerPosition(x, y)
  lib.microListenerSetPosition(x, y)
end

---@param distance number
function Audio.setListenerMaxDistance(distance)
  lib.microListenerSetMaxDistance(distance)
end

return Audio
