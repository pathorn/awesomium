
ifeq ($(CHROMIUMDIR),)
CHROMIUMDIR=../../../chromium/chromium
endif

ifeq ($(CHROMIUMMODE),)
CHROMIUMMODE=Release
endif

ifeq ($(CHROMIUMMODE),Debug)
DEBUGFLAGS=-D_DEBUG
else
DEBUGFLAGS=-DNDEBUG
endif

ifeq ($(ARCHFLAGS),)
ARCHFLAGS=-m32
endif

ifeq ($(CHROMIUMBUILDER),make)
#### make build system

CHLIBS=$(CHROMIUMLIBPATH)
ifeq ($(CHLIBS),)
CHLIBS=$(CHROMIUMDIR)/src/out/$(CHROMIUMMODE)/obj
endif

LIBDIRS=. app base ipc chrome net media webkit sandboxwebkit skia printing v8/tools/gyp sdch build/temp_gyp
THIRDPARTYLIBDIRS=bzip2 ffmpeg harfbuzz hunspell icu libevent libjpeg libpng libxml libxslt lzma_sdk modp_b64 sqlite zlib

CHROMIUMLDFLAGS=$(addprefix -L$(CHLIBS)/,$(LIBDIRS)) $(addprefix -L$(CHLIBS)/third_party/,$(THIRDPARTYLIBDIRS))
TPLIBS=-llibevent -lzlib -llibpng -llibxml -llibjpeg -llibxslt -lbzip2

else
#### scons build system

ifeq ($(CHROMIUMLIBPATH),)
CHROMIUMLIBPATH=$(CHROMIUMDIR)/src/sconsbuild/$(CHROMIUMMODE)/lib
endif

CHROMIUMLDFLAGS=-L$(CHROMIUMLIBPATH)
TPLIBS=-levent -lxslt -ljpeg -lpng -lz -lxml2 -lbz2
endif
CHROMIUMLIBS=$(CHROMIUMLDFLAGS) $(TPLIBS) -lsmime3 -lplds4 -lplc4 -lnspr4 -lpthread -ldl -lgdk-x11-2.0 -lgdk_pixbuf-2.0 -lm -lpangocairo-1.0 -lgio-2.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lfontconfig -lfreetype -lrt -lgconf-2 -lglib-2.0 -lX11 -lasound -lcommon -lbrowser -ldebugger -lrenderer -lutility -lprinting -lapp_base -lbase -licui18n -licuuc -lbase_gfx -lskia -llinux_versioninfo -lharfbuzz -lharfbuzz_interface -lnet -lgoogleurl -lsdch -lmodp_b64 -lv8_snapshot -lv8_base -lglue -lwebcore -lpcre -lwtf -lsqlite -lwebkit -lmedia -lffmpeg -lhunspell -lplugin -lappcache -lipc -licudata

# Flags that affect both compiling and linking
CLIBFLAGS=$(ARCHFLAGS) -fvisibility=hidden -fvisibility-inlines-hidden -fPIC -pthread -Wall -fno-rtti

CFLAGS=$(DEBUGFLAGS) $(CLIBFLAGS) -Wall -D_REENTRANT -D__STDC_FORMAT_MACROS -DCHROMIUM_BUILD -DU_STATIC_IMPLEMENTATION -g -I ../include `pkg-config --cflags gtk+-2.0 glib-2.0 gio-unix-2.0` $(addprefix -I$(CHROMIUMDIR)/src/,. third_party/npapi third_party/WebKit/JavaScriptCore deps/third_party/icu42/public/common skia/config third_party/skia/include/core webkit/api/public third_party/WebKit/WebCore/platform/text)

LIBS=-shared $(CLIBFLAGS) -g -Wl,--start-group `pkg-config --libs gtk+-2.0 glib-2.0 gio-unix-2.0 gconf-2.0` -lssl3 -lnss3 -lnssutil3 $(CHROMIUMLIBS) -Wl,--end-group

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)
HEADERS=$(wildcard ../include/*.h)

TARGET=../lib/libawesomium.so

#all: mymain

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir ../lib 2>/dev/null || true
	g++ $(OBJS) $(LIBS) -o $@

%.o: %.cpp $(HEADERS)
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

mymain: $(OBJS)
	g++ $(CFLAGS) $(OBJS) $(LIBS) -o mymain

