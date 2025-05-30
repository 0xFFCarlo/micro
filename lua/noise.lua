local ffi = require("ffi")
local lib = require("micro/lua/libmicro")

ffi.cdef([[
    typedef struct NoiseCfg
    {
      int width;
      int height;
      float frequency;
      float amplitude;
      float offsetX;
      float offsetY;
      int octaves;
      float persistence;
      float lacunarity;
      bool usePeriodic;
      int periodX;
      int periodY;
      bool normalize;
    } NoiseCfg;

    void microNoisePelin2(float *data, const NoiseCfg cfg);
]])

local Noise = {}

---This function generates a 2D noise map
---using the Perlin noise algorithm.
---@param width number
---@param height number
---@param frequency number
---@param amplitude number
---@param offsetX number
---@param offsetY number
---@param octaves number
---@param persistence number
---@param lacunarity number
---@param usePeriodic boolean
---@param periodX number
---@param periodY number
---@param normalize boolean
function Noise.generate(
	width,
	height,
	frequency,
	amplitude,
	offsetX,
	offsetY,
	octaves,
	persistence,
	lacunarity,
	usePeriodic,
	periodX,
	periodY,
	normalize
)
	local cfg = ffi.new("NoiseCfg")
	cfg.width = width
	cfg.height = height
	cfg.frequency = frequency
	cfg.amplitude = amplitude
	cfg.offsetX = offsetX
	cfg.offsetY = offsetY
	cfg.octaves = octaves
	cfg.persistence = persistence
	cfg.lacunarity = lacunarity
	cfg.usePeriodic = usePeriodic
	cfg.periodX = periodX
	cfg.periodY = periodY
	cfg.normalize = normalize

	local dataPtr = ffi.new("float[?]", width * height)

	lib.microNoisePelin2(dataPtr, cfg)

	return dataPtr
end

return Noise
