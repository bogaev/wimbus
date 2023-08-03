# 🚍 WimBus

**WimBus** (*Where Is My Bus?*) — транспортный справочник, позволяющий узнать кратчайший маршрут между двумя остановками. Программа позволяет:

- Вводить новые остановки и маршруты автобусов в формате json
- Сохранять созданую базу данных в оптимизированном по размеру двоичном формате
- Искать кратчайший маршрут между двумя остановками с учетом пересадок
- Выводить карту маршрутов в файл формата xml

## ⚙️ Системные требования

- C++17 standard
- STL [^1]
- Cmake [^2] minimum version: 3.10
- Protobuf [^3]

## 🛠️ Сборка программы в Ubuntu

#### <img src="https://plugins.jetbrains.com/files/14004/364074/icon/pluginIcon.png" alt="java" width="20"/> Установка Protobuf

- Установите Protobuf с помощью apt

	```
	sudo apt update
	sudo apt install protobuf-compiler
	protoc --version
	```

#### <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/13/Cmake.svg/1200px-Cmake.svg.png" alt="java" width="20"/> Сборка с помощью Cmake

- Установите cmake

	```
	sudo apt install cmake
	```

- Соберите приложение wimbus с помощью cmake в директории build:

	```
	cd [path-to-cpp-wimbus-main]\
	mkdir build && cd build
	cmake .. && make
	```

## 🛣 Использование WimBus

При запуске без параметров ```./wimbus``` программа подскажет 2 основных режима своей работы:

```
./wimbus
Usage: transport_catalogue [make_base|process_requests]
```

- Режим создания базы данных маршрутов ```make_base``` с сохранением в файл
- Режим обработки запросов к базе ```process_requests``` с возможностью вывода карты маршрутов в формате xml

#### 🗄 Создание базы данных остановок и маршрутов

При запуске программы с параметром ```./wimbus make_base``` пользователь может сконфигурировать базу, ввести остановки и маршруты в формате json:

<details>
<summary>Пример запроса ([⬇️ make_base_example](request_examples/make_base_example))</summary>

```
{
    "serialization_settings": {             // установки сериализации базы данных
        "file": "transport_catalogue.db"    // имя файла, в который будет сохранена готовая база остановок и маршрутов
    },
    "routing_settings": {                   // установки автобусов
        "bus_wait_time": 2,                 // время ожидания автобуса на остановке
        "bus_velocity": 30                  // средняя скорость автобуса
    },
    "render_settings": {                    // установки отрисовки карты маршрутов и остановок
        "width": 1200,                      // ширина карты
        "height": 500,                      // высота карты
        "padding": 50,                      // отступы
        "stop_radius": 5,                   // радиус остановки
        "line_width": 14,                   // ширина линий маршрутов
        "bus_label_font_size": 20,          // размер шрифта названий маршрутов
        "bus_label_offset": [               // отступы названий маршрутов
            7,
            15
        ],
        "stop_label_font_size": 18,         // размер шрифта названий остановок
        "stop_label_offset": [              // отступы названий остановок
            7,
            -3
        ],
        "underlayer_color": [               // цвет подложки карты
            255,                            // R
            255,                            // G
            255,                            // B
            0.85                            // alpha
        ],
        "underlayer_width": 3,              
        "color_palette": [                  // цветовая палитра для отрисовки маршрутов
            "green",                        // цвет в строковом формате
            [
                255,                        // R
                160,                        // G
                0                           // B
            ],
            "red"
        ]
    },
    "base_requests": [                      // запросы к базе на добавление маршрутов (автобусов)
        {
            "type": "Bus",                  // запрос на добавление автобуса
            "name": "14",                   // название маршрута
            "stops": [                      // список остановок маршрута
                "Улица Лизы Чайкиной",
                "Электросети",
                "Ривьерский мост",
                "Гостиница Сочи",
                "Кубанская улица",
                "По требованию",
                "Улица Докучаева",
                "Улица Лизы Чайкиной"
            ],
            "is_roundtrip": true            // наличие кольцевого движения по маршруту
        },
        {
            "type": "Bus",
            "name": "24",
            "stops": [
                "Улица Докучаева",
                "Параллельная улица",
                "Электросети",
                "Санаторий Родина"
            ],
            "is_roundtrip": false
        },
        {
            "type": "Bus",
            "name": "114",
            "stops": [
                "Морской вокзал",
                "Ривьерский мост"
            ],
            "is_roundtrip": false
        },
        {
            "type": "Stop",                 // запрос на добавление остановки
            "name": "Улица Лизы Чайкиной",  // название остановки
            "latitude": 43.590317,          // широта
            "longitude": 39.746833,         // долгота
            "road_distances": {             // расстояния до других остановок в метрах
                "Электросети": 4300,
                "Улица Докучаева": 2000
            }
        },
        {
            "type": "Stop",
            "name": "Морской вокзал",
            "latitude": 43.581969,
            "longitude": 39.719848,
            "road_distances": {
                "Ривьерский мост": 850
            }
        },
        {
            "type": "Stop",
            "name": "Электросети",
            "latitude": 43.598701,
            "longitude": 39.730623,
            "road_distances": {
                "Санаторий Родина": 4500,
                "Параллельная улица": 1200,
                "Ривьерский мост": 1900
            }
        },
        {
            "type": "Stop",
            "name": "Ривьерский мост",
            "latitude": 43.587795,
            "longitude": 39.716901,
            "road_distances": {
                "Морской вокзал": 850,
                "Гостиница Сочи": 1740
            }
        },
        {
            "type": "Stop",
            "name": "Гостиница Сочи",
            "latitude": 43.578079,
            "longitude": 39.728068,
            "road_distances": {
                "Кубанская улица": 320
            }
        },
        {
            "type": "Stop",
            "name": "Кубанская улица",
            "latitude": 43.578509,
            "longitude": 39.730959,
            "road_distances": {
                "По требованию": 370
            }
        },
        {
            "type": "Stop",
            "name": "По требованию",
            "latitude": 43.579285,
            "longitude": 39.733742,
            "road_distances": {
                "Улица Докучаева": 600
            }
        },
        {
            "type": "Stop",
            "name": "Улица Докучаева",
            "latitude": 43.585586,
            "longitude": 39.733879,
            "road_distances": {
                "Параллельная улица": 1100
            }
        },
        {
            "type": "Stop",
            "name": "Параллельная улица",
            "latitude": 43.590041,
            "longitude": 39.732886,
            "road_distances": {}
        },
        {
            "type": "Stop",
            "name": "Санаторий Родина",
            "latitude": 43.601202,
            "longitude": 39.715498,
            "road_distances": {}
        }
    ]
}
```
</details>

Данный запрос запишет конфигурацию карты, маршруты и остановки в файл ```transport_catalogue.db```

#### 🧑🏻‍💻 Запросы к готовой базе данных

При запуске программы с параметром ```./wimbus process_requests``` пользователь может запросить информацию об оптимальных маршрутах из существующей базы данных:

<details>
<summary>Пример запроса ([⬇️ process_requests_example](request_examples/process_requests_example))</summary>

```
{
    "serialization_settings": {             // настройки сериализации базы данных
        "file": "transport_catalogue.db"    // имя файла базы данных маршрутов
    },
    "stat_requests": [                      // массив запросов к БД
        {
            "id": 218563507,                // идентификатор запроса
            "type": "Bus",                  // тип запроса
            "name": "14"                    // название маршрута
        },
        {
            "id": 508658276,
            "type": "Stop",
            "name": "Электросети"           // название остановки
        },
        {
            "id": 1964680131,
            "type": "Route",                // расчет оптимального маршрута
            "from": "Морской вокзал",       // начальная остановка
            "to": "Параллельная улица"      // конечная остановка
        },
        {
            "id": 1359372752,
            "type": "Map"                   // запрос на генерацию карты остановок и маршрутов
        }
    ]
}
```
</details>

Данный запрос:
- Распакует базу ```transport_catalogue.db```
- Выведет параметры маршрута: ```stat_requests: "Bus"```
- Выведет информацию об остановке: ```stat_requests: "Stop"```
- Рассчитает оптимальный маршрут: ```stat_requests: "Route"```
- Сгенерирует карту формате xml/svg: ```stat_requests: "Map"```

<details>
<summary>Выходные данные ([⬇️ requests_output_example](request_examples/requests_output_example))</summary>

```
[
    {                                           // характеристики маршрута
        "curvature": 1.60481,                   // кривизна (коэффициент отклонения местности от плоскости)
        "request_id": 218563507,                // идентификатор запроса
        "route_length": 11230,                  // длина маршрута в метрах
        "stop_count": 8,                        // количество остановок
        "unique_stop_count": 7                  // количество уникальных остановок
    },
    {
        "buses": [                              // маршруты (автобусы), которые проходят через данную остановку
            "14",                               // названия автобусов
            "24"
        ],
        "request_id": 508658276                 // идентификатор запроса
    },
    {
        "items": [                              // элементы оптимального маршрута
            {
                "stop_name": "Морской вокзал",  // название остановки
                "time": 2,                      // время ожидания на остановке
                "type": "Wait"                  // тип элемента (ожидание на остановке)
            },
            {
                "bus": "114",                   // название маршрута
                "span_count": 1,                // количество остановок которое нужно проехать
                "time": 1.7,                    // продолжительность маршрута в минутах
                "type": "Bus"                   // тип элемента (проезд на автобусе)
            },
            {
                "stop_name": "Ривьерский мост",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "14",
                "span_count": 4,
                "time": 6.06,
                "type": "Bus"
            },
            {
                "stop_name": "Улица Докучаева",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "24",
                "span_count": 1,
                "time": 2.2,
                "type": "Bus"
            }
        ],
        "request_id": 1964680131,               // идентификатор запроса на вычисление маршрута
        "total_time": 15.96                     // общее время оптимального маршрута
    },
    {   // карта остановок и маршрутов в формате xml/svg
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"125.25,382.708 74.2702,281.925 125.25,382.708\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"592.058,238.297 311.644,93.2643 74.2702,281.925 267.446,450 317.457,442.562 365.599,429.138 367.969,320.138 592.058,238.297\" fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"367.969,320.138 350.791,243.072 311.644,93.2643 50,50 311.644,93.2643 350.791,243.072 367.969,320.138\" fill=\"none\" stroke=\"red\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgb(255,160,0)\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"red\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"red\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <circle cx=\"267.446\" cy=\"450\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"317.457\" cy=\"442.562\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"125.25\" cy=\"382.708\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"350.791\" cy=\"243.072\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"365.599\" cy=\"429.138\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"74.2702\" cy=\"281.925\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"50\" cy=\"50\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"367.969\" cy=\"320.138\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"592.058\" cy=\"238.297\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"311.644\" cy=\"93.2643\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Гостиница Сочи</text>\n  <text fill=\"black\" x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Гостиница Сочи</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Кубанская улица</text>\n  <text fill=\"black\" x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Кубанская улица</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Параллельная улица</text>\n  <text fill=\"black\" x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Параллельная улица</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">По требованию</text>\n  <text fill=\"black\" x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">По требованию</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Санаторий Родина</text>\n  <text fill=\"black\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Санаторий Родина</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Докучаева</text>\n  <text fill=\"black\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Докучаева</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n  <text fill=\"black\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Электросети</text>\n  <text fill=\"black\" x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Электросети</text>\n</svg>",
        "request_id": 1359372752 // идентификатор запроса на генерацию карты
    }
]
```
</details>

<!--
## Примеры
-->

[^1]: https://en.wikipedia.org/wiki/Standard_Template_Library
[^2]: https://cmake.org/download/
[^3]: https://github.com/protocolbuffers/protobuf