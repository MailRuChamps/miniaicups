**Требования**
* Python 3

**Использование**

Запуск через `python3 converter.py path_to_log path_to_json_log`

После парсинга `json` файл будет содержать в себе только разницу между тиками, чтобы получить полный лог необходимо запустить парсер с флагом -f первым параметром `python3 converter.py -f path_to_log path_to_json_log`.

**Формат**
```
{
    "config": {
        "config_key": config_param,
    },
    "ticks_delta": {  // or "ticks" for full log
        "tick_num": {
            "c" {  // commands
                "player_id": {
                    "x": double,
                    "y": double,
                    "s": boolean, // split
                    "e": boolean  // eject
                },
            },
            "v": {  // viruses
                "id":{
                    "x": double,
                    "y": double,
                    "s": double,  // speed (optional)
                    "a": double   // angle (optional)
                },
            },
            "p": {  // players
                "id": {
                    "x": double,
                    "y": double,
                    "m": double,  // mass
                    "f": double,  // fog
                    "c": int,     // color code (ignore it)
                    "r": double,  // radius
                    "s": double,  // speed (not available in zero tick)
                    "a": double,  // angle (not available in zero tick)
                },
            },
            "s" {  // diff scores
                "player_id": int
            },
            "f": {  // food
                "id": {
                    "x": double,
                    "y": double
                }
            },
            "e": {  // ejects
                "id": {
                    "x": double,
                    "y": double,
                    "p": int,     // owner player id
                    "s": double,  // speed
                    "a": double,  // angle
                }
            },
            "dv": [ // deleted viruses
                id, id
            ],
            "de": [ // deleted ejects
                id, id
            ],
            "dp": [ // deleted players
                id, id
            ],
            "df": [ // deleted food
                id, id
            ]
        },
    },
}
```

**Пример вывода**

```
{
    "config": {
        "VIRUS_SPLIT_MASS": 77.78719617615721,
        "PLAYER_MASS": 40,
        "SPEED_FACTOR": 63.96458505835971,
        "GAME_TICK": 7500,
        "GAME_WIDTH": 990,
        "GAME_HEIGHT": 990,
        "PLAYER_RADIUS": 12.649110640673518,
        "EJECT_MASS": 15,
        "VIRUS_RADIUS": 33.11477048699801,
        "VIRUS_MASS": 40,
        "FOOD_MASS": 2.903647064693688,
        "VISCOSITY": 0.09614842784128402,
        "FOOD_RADIUS": 2.5,
        "TICKS_TIL_FUSION": 444,
        "MAX_FRAGS_CNT": 12,
        "INERTION_FACTOR": 5.775299008673391,
        "EJECT_RADIUS": 4
    },
    "ticks_delta": {
        "0": {
             "c": {
                 "2": {
                     "y": 72.8002252623841,
                     "e": false,
                     "x": 844.802003022824,
                     "s": false
                 }, ...
             }
             "v": {
                 "24": {
                     "y": 781.770459026004,
                     "x": 79.22954097399602
                 },
                 "21": {
                     "y": 208.229540973996,
                     "x": 79.22954097399602
                 }, ...
             },
             "p": {
                "2": {
                    "m": 40,
                    "y": 34.29822128134704,
                    "f": 50.59644256269407,
                    "c": 9,
                    "x": 876.701778718653,
                    "r": 12.64911064067352
                }, ...
            },
            "s": {
                "1": 0,
                "2": 0,
                "4": 0,
                "3": 0
            },
            "f": {
                "5": {
                    "y": 122,
                    "x": 192
                },
                "6": {
                    "y": 122,
                    "x": 798
                }, ...
            },
            "e": {},
            "dv": [],
            "de": [],
            "dp": [],
            "df": []
        }, ...
    }
}
```
