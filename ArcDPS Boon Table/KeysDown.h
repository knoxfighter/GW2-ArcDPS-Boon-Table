#pragma once

class KeysDown
{
public:
    static void SetKeyDown(int key, bool down)
    {
        if (down)
        {
            if (keys[key] < 255)
            {
                keys[key]++;
            }
        }
        else
        {
            keys[key] = 0;
        }
    }

    static bool IsKeyDown(int key)
    {
        return keys[key] != 0;
    }

    static bool IsKeyPressed(int key)
    {
        return keys[key] == 1;
    }

    static size_t Size()
    {
        return sizeof(keys);
    }
private:
    inline static uint8_t keys[512] = {};
};
