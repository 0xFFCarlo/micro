function sleep(n)
    os.execute("sleep " .. tonumber(n))
end

print("SETUP\n");
print("Loading sound effect...\n");
local sound = SoundEffect("res/test.wav");
print("Loaded sound effect\n");

print("TEST 1\n");
print("Playing sound effect fully...\n");
sound:play(0);
while sound:isPlaying() do
    sleep(0.05);
end

print("TEST 2\n");
print("Playing sound effect looping...\n");
sound:play(-1);
sleep(6);

print("TEST 3\n");
print("Playing sound effect and stopping it...\n");
sound:play(0);
sleep(0.7);
sound:stop();
print("Sound stopped \n");
sleep(2);

print("TEST 4\n");
print("Playing sound effect pausing for 1 sec and resuming...\n");
sound:play(0);
sleep(0.7);
sound:pause();
print("Sound paused \n");
sleep(1);
sound:resume();
print("Sound resumed \n");
sleep(2);

print("TEST 5\n");
print("Playing sound and change voume\n");
sound:play(0);
sleep(0.5);
sound:setVolume(0.5);
print("Volume halved \n");
sleep(0.5);
sound:setVolume(1.0);
print("Volume restored \n");
sleep(2);

print("FREE\n");
print("Unloading sound effect...\n");
sound = nil;
print("Unloaded sound effect\n");
sleep(1);

print("SETUP\n");
print("Loading music...\n");
local music = Music("res/music.mp3");
print("Loaded music\n");

print("Playing music...\n");
music:play(0);
sleep(1);
print("Is music playing? -> " .. tostring(music:isPlaying()));
sleep(1);
music:setVolume(0.5);
print("Volume halved \n");
sleep(2);
music:setVolume(1.0);
print("Volume restored \n");
sleep(1);
music:pause();
print("Music paused \n");
sleep(2);
music:resume();
print("Music resumed \n");
sleep(2);
music:stop();
print("Music stopped \n");
sleep(2);