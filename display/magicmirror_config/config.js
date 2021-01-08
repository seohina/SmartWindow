var config = {
  address: "localhost",
  port: 8080,
  ipWhitelist: ["127.0.0.1", "::ffff:127.0.0.1", "::1"],
  language: "ko",
  timeFormat: 12,
  units: "metric",

  modules: [
    {
      module: "alert",
    },
    {
      module: "updatenotification",   #업데이트 알림 모듈
      position: "top_bar"
    },
    {
      module: "clock",        # 시계 모듈
      position: "top_left",
      config: {
        dateFormat: "LL dddd",
      }
    },
   
    {
      module: "currentweather",   # 현재 날씨 모듈 (OpenWeather API 사용)
      position: "top_right",
      config: {
        location: "Seoul, KR",     # 위치 설정
        locationID: "",
        appid: "ee7f12b34aff09e5b1e5e1e83b93f723",   #API키 입력
   }
},
{
      module: "MMM-WeatherDependentClothes",   #온도별 옷차림 모듈
      position: "middle_center",
      config: {
        location: "Seoul, KR",    # 날씨 정보를 가져올 위치 설정

        locationID: "",
        appid: "ee7f12b34aff09e5b1e5e1e83b93f723",   # API키 입력
   preferences: [
        {
            name: "Freezing",
            icon: "cold",
            conditions: {
               temp_max: 0,
            }
         },
         {
            name: "HeavyRain",
            icon: "wet",
            conditions: {
               rainfall_min: 14,
            }
         },
         {
            name: "HeavyWind",
            icon: "wind",
            conditions: {
               windSpeed_min: 10,
            }
         },
         {
            name: "Sandalen",
            icon: "sandals",
            conditions: {
               temp_min: 28.0,
            }
         },
         {
            name: "Schuhe",
            icon: "shoes",
            conditions: {
               temp_min: 0.0,
               temp_max: 28.0,
            }
         },
         {
            name: "warme Hose",
            icon: "pants-cold",
            conditions: {
               temp_max: 8.0,
            },
         },
         {
            name: "lange Hose",
            icon: "pants",
            conditions: {
               temp_min: 8.0,
               temp_max: 16.0,
            },
         },
         {
            name: "kurze Hose",
            icon: "shorts",
            conditions: {
               temp_min: 16.0,
            },
         },
                  {
            name: "Winterjacke",
            icon: "jacket-cold",
            conditions: {
               temp_max: 2.0,
            },
         },
         {
            name: "Jacke",
            icon: "jacket",
            conditions: {
               temp_min: 2.0,
               temp_max: 9.0,
               rainfall_max: 3,
            },
         },
         {
            name: "Regenjacke",
            icon: "jacket-wet",
            conditions: {
               temp_min: 2.0,
               temp_max: 9.0,
               rainfall_min: 3,
            },
         },
         {
            name: "Pullover",
            icon: "hoodie",
            conditions: {
               temp_min: 9.0,
               temp_max: 14.0,
            },
         },
         {
            name: "langes Shirt",
            icon: "shirt",
            conditions: {
               temp_min: 14.0,
               temp_max: 17.0,
            },
         },
         {
            name: "kurzes T-Shirt",
            icon: "shirt-t",
            conditions: {
               temp_min: 17.0,
            },
         },
         {
            name: "Regenschirm",
            icon: "umbrella2",
            conditions: {
               rainfall_min: 4, // mm
            },
         },
         {
            name: "Sonnenbrille",
            icon: "sunglasses",
            conditions: {
               weather: "clear",
            },
         },
         {
            name: "Sonnenbrille",
            icon: "sunglasses",
            conditions: {
               weather: "clouds",
               cloudDensity_max: 50,
            },
         },
         {
            name: "Hut",
            icon: "hat",
            conditions: {
               windSpeed_min: 6.0,
            }
         },
]}
},
    {
      module: "weatherforecast",   # 날씨 예보 모듈
      position: "lower_third",
      config: {
        location: "Seoul, KR",    # 위치 설정
        locationID: "",
        appid: "ee7f12b34aff09e5b1e5e1e83b93f723",    # API 입력
        showRainAmount: "true",    # 예상 강우량 표시
   colored: "true"
      }
    },
{
   module: 'MMM-AirQuality',   # 현재 대기상태 모듈
   position: 'top_right', 
   config: {
     location: 'anyang-si',   #위치 설정

   }
},

{
         module: "newsfeed",      # 뉴스피드 모듈
         position: "bottom_bar",
         config: {
            feeds: [
               {
                  title: "JTBC",
                  url: "http://fs.jtbc.joins.com/RSS/newsflash.xml“
                  # JTBC뉴스 속보 xml 파싱
               }
            ],
            showSourceTitle: true,    # 뉴스 제목표시
            showPublishDate: true    # 헤드 라인의 게시 날짜를 표시
         }
      }, 

    {
      module: "compliments",   # 출력할 텍스트를 설정하는 모듈
      position: "top_left",
      config: {
        compliments: {
          anytime: [ 
          ],
          morning: [ 
          ],
          afternoon: [ 
          ],
          evening: [ 
          ],
          day_sunny: [
          ],
          day_cloudy: [
          ],
          cloudy: [
          ],
          cloudy_windy: [
          ],
          showers: [   # 날씨 상태가 소나기일 때 우산알림 출력
          "\n우산!"

          ],
          rain: [     # 날씨 상태가 비일 때 우산알림 출력
           "\n우산!"
          ],
          thunderstorm: [
          ],
          snow: [
          ],
          fog: [
          ],
          night_clear: [
          ],
          night_cloudy: [
          ],
          night_showers: [//소나기 밤

          ],
          night_rain: [    # 날씨 상태가 비오는 밤일 때 우산알림 출력
           "\n우산!"
          ],
          night_thunderstorm: [
          ],
          night_snow: [
          ],
          night_alt_cloudy_windy: [
          ],
        }
      }
    },

    {
      module: "MMM-AssistantMk2",
      position: "top_left",
      config: {
        ui: "Fullscreen",
        assistantConfig: {
          latitude: 37.57,
          longitude: 126.98
        },
        micConfig: {
          recoder : "arecord",  
          device  : "plughw:1",
        },
        recipes: [ 
          "with-MMM-Hotword.js",
          "with-MMM-Youtube.js",
          ],
        profiles: {
          "default": {
            lang: "ko-KR"
          }
        },
        addons: false,
      }
    },
    {
      module: "MMM-YouTube",
      position: "top_right"
    },
    {
      module: "MMM-Hotword",
      position: "",
      config: {
        chimeOnFinish: null,
        recipes: [ "with-AMk2v3_smart-mirror.js" ],
        mic: {
          recordProgram: "arecord",
          device: "plughw:1"
        }
      }
    },
  ]


};

/*************** DO NOT EDIT THE LINE BELOW ***************/
if (typeof module !== "undefined") {
  module.exports = config;
}