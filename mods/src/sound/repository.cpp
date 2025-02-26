#include <string>
#include <cstdlib>
#include <ctime>

#include <libreborn/log.h>
#include <media-layer/audio.h>

#include "internal.h"
#include <mods/sound/sound.h>

// Sound Repository Extracted From MCPE 0.6.1 APK
std::unordered_map<std::string, std::vector<std::string>> sound_repository = {
    {
        {
            "step.cloth",
            {
                "PCM_cloth1",
                "PCM_cloth2",
                "PCM_cloth3",
                "PCM_cloth4"
            }
        },
        {
            "step.grass",
            {
                "PCM_grass1",
                "PCM_grass2",
                "PCM_grass3",
                "PCM_grass4"
            }
        },
        {
            "step.gravel",
            {
                "PCM_gravel1",
                "PCM_gravel2",
                "PCM_gravel3",
                "PCM_gravel4"
            }
        },
        {
            "step.sand",
            {
                "PCM_sand1",
                "PCM_sand2",
                "PCM_sand3",
                "PCM_sand4"
            }
        },
        {
            "step.stone",
            {
                "PCM_stone1",
                "PCM_stone2",
                "PCM_stone3",
                "PCM_stone4"
            }
        },
        {
            "step.wood",
            {
                "PCM_wood1",
                "PCM_wood2",
                "PCM_wood3",
                "PCM_wood4"
            }
        },
        {
            "random.splash",
            {
                "PCM_splash"
            }
        },
        {
            "random.explode",
            {
                "PCM_explode"
            }
        },
        {
            "random.click",
            {
                "PCM_click"
            }
        },
        {
            "random.door_open",
            {
                "PCM_door_open"
            }
        },
        {
            "random.door_close",
            {
                "PCM_door_close"
            }
        },
        {
            "random.pop",
            {
                "PCM_pop"
            }
        },
        {
            "random.pop2",
            {
                "PCM_pop2"
            }
        },
        {
            "random.hurt",
            {
                "PCM_hurt"
            }
        },
        {
            "random.glass",
            {
                "PCM_glass1",
                "PCM_glass2",
                "PCM_glass3"
            }
        },
        {
            "mob.sheep",
            {
                "PCM_sheep1",
                "PCM_sheep2",
                "PCM_sheep3"
            }
        },
        {
            "mob.pig",
            {
                "PCM_pig1",
                "PCM_pig2",
                "PCM_pig3"
            }
        },
        {
            "mob.pigdeath",
            {
                "PCM_pigdeath"
            }
        },
        {
            "mob.cow",
            {
                "PCM_cow1",
                "PCM_cow2",
                "PCM_cow3",
                "PCM_cow4"
            }
        },
        {
            "mob.cowhurt",
            {
                "PCM_cowhurt1",
                "PCM_cowhurt2",
                "PCM_cowhurt3"
            }
        },
        {
            "mob.chicken",
            {
                "PCM_chicken2",
                "PCM_chicken3"
            }
        },
        {
            "mob.chickenhurt",
            {
                "PCM_chickenhurt1",
                "PCM_chickenhurt2"
            }
        },
        {
            "mob.zombie",
            {
                "PCM_zombie1",
                "PCM_zombie2",
                "PCM_zombie3"
            }
        },
        {
            "mob.zombiedeath",
            {
                "PCM_zombiedeath"
            }
        },
        {
            "mob.zombiehurt",
            {
                "PCM_zombiehurt1",
                "PCM_zombiehurt2"
            }
        },
        {
            "mob.skeleton",
            {
                "PCM_skeleton1",
                "PCM_skeleton2",
                "PCM_skeleton3"
            }
        },
        {
            "mob.skeletonhurt",
            {
                "PCM_skeletonhurt1",
                "PCM_skeletonhurt2",
                "PCM_skeletonhurt3",
                "PCM_skeletonhurt4"
            }
        },
        {
            "mob.spider",
            {
                "PCM_spider1",
                "PCM_spider2",
                "PCM_spider3",
                "PCM_spider4"
            }
        },
        {
            "mob.spiderdeath",
            {
                "PCM_spiderdeath"
            }
        },
        {
            "mob.zombiepig.zpig",
            {
                "PCM_zpig1",
                "PCM_zpig2",
                "PCM_zpig3",
                "PCM_zpig4"
            }
        },
        {
            "mob.zombiepig.zpigangry",
            {
                "PCM_zpigangry1",
                "PCM_zpigangry2",
                "PCM_zpigangry3",
                "PCM_zpigangry4"
            }
        },
        {
            "mob.zombiepig.zpigdeath",
            {
                "PCM_zpigdeath"
            }
        },
        {
            "mob.zombiepig.zpighurt",
            {
                "PCM_zpighurt1",
                "PCM_zpighurt2"
            }
        },
        {
            "damage.fallbig",
            {
                "PCM_fallbig1",
                "PCM_fallbig2"
            }
        },
        {
            "damage.fallsmall",
            {
                "PCM_fallsmall"
            }
        },
        {
            "random.bow",
            {
                "PCM_bow"
            }
        },
        {
            "random.bowhit",
            {
                "PCM_bowhit1",
                "PCM_bowhit2",
                "PCM_bowhit3",
                "PCM_bowhit4"
            }
        },
        {
            "mob.creeper",
            {
                "PCM_creeper1",
                "PCM_creeper2",
                "PCM_creeper3",
                "PCM_creeper4"
            }
        },
        {
            "mob.creeperdeath",
            {
                "PCM_creeperdeath"
            }
        },
        {
            "random.eat",
            {
                "PCM_eat1",
                "PCM_eat2",
                "PCM_eat3"
            }
        },
        {
            "random.fuse",
            {
                "PCM_fuse"
            }
        },
        {
            "random.burp",
            {
                "PCM_burp"
            }
        },
        {
            "random.fizz",
            {
                "PCM_fizz"
            }
        },
        {
            "random.drink",
            {
                "PCM_drink"
            }
        },
        {
            "random.chestopen",
            {
                "PCM_chestopen"
            }
        },
        {
            "random.chestclosed",
            {
                "PCM_chestclosed"
            }
        }
    }
};

// Set rand() Seed
__attribute__((constructor)) static void init_rand_seed() {
    srand(time(nullptr));
}
// Pick Sound
std::string _sound_pick(const std::string &sound) {
    if (sound_repository.contains(sound)) {
        // Sound Exists
        std::vector<std::string> &options = sound_repository[sound];
        return options[rand() % options.size()];
    } else {
        // Invalid Sound
        WARN("Invalid Sound Event: %s", sound.c_str());
        return "";
    }
}

// Resolve All Sounds
void _sound_resolve_all() {
    const std::string source = _sound_get_source_file();
    if (!source.empty()) {
        for (const std::pair<const std::string, std::vector<std::string>> &it : sound_repository) {
            for (const std::string &name : it.second) {
                // Zero Volume Prevents An OpenAL Source From Being Allocated While Still Resolving The Sound
                media_audio_play(source.c_str(), name.c_str(), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
            }
        }
    }
}
