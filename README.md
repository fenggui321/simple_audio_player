# miplayer 

# depends

  asound
  
  ffmpeg libavcodec libavformat libavutil libavresample  3.4
  
  SDL2    libSDL2
  
  json-c https://github.com/json-c/json-c.git   0.13
  
  mosquitto https://github.com/eclipse/mosquitto.git 
  

# 编译依赖库

## FFmpeg compile param

```

./configure --enable-version3 --disable-x86asm --enable-shared --disable-postproc --disable-swscale --disable-avfilter 
         --disable-swscale --disable-avdevice --disable-filters --disable-iconv  --enable-openssl --enable-avresample
         
```

debug param: --enable-debug=3 --disable-stripping 

  openssl 为支持https 播放资源

  zlib(zip)为支持http压缩



# build miplayer

cd player

make

sudo ./run.sh 

# testCase：

#offset 为pause/stop位置,此参数控制播放器seek开始播放位置

#url:http://files.ai.xiaomi.com/aiservice/xiaoai/49f31ff28914b8f294b7a6e388609517.mp3

```
mosquitto_pub -d  -t "player" -m  "{\"sid\":1,\"op\":\"start\",\"url\":\"chineseMan.mp3\", \"offset\":0}"

mosquitto_pub -d  -t "player" -m  "{\"sid\":1,\"op\":\"start\",\"url\":\"chineseMan.mp3\", \"offset\":90}"

mosquitto_pub -d  -t "player" -m  "{\"sid\":1,\"op\":\"pause\"}"

mosquitto_pub -d  -t "player" -m  "{\"sid\":1,\"op\":\"resume\"}"

mosquitto_pub -d  -t "player" -m  "{\"sid\":1,\"op\":\"stop\"}"

mosquitto_pub -d  -t "player" -m  "{\"sid\":1,\"op\":\"speak\",\"url\":\"chineseMan.mp3\", \"offset\":50}"

mosquitto_pub -d  -t "player" -m  "{\"sid\":1,\"op\":\"debug\",\"debuglevel\":7}"

```
