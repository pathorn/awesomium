
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

ifeq ($(CHROMIUMLIBPATH),)
CHROMIUMLIBPATH=$(CHROMIUMDIR)/src/sconsbuild/$(CHROMIUMMODE)/lib
endif

#CHROMIUMLIBNAMES=libbrowser.a libbz2.a libcommon.a libwebcore.a libdebugger.a libgoogleurl.a libgtest.a libharfbuzz.a libharfbuzz_interface.a libhunspell.a libmedia.a libplugin.a libprinting.a librenderer.a libutility.a libglue.a libnet.a libsdch.a libv8.a libv8_base.a libv8.a libwebkit.a libwebcore.a libskia.a libglue.a libwebcore.a libapp_base.a libbase.a libbase_gfx.a libevent.a libffmpeg.a libwtf.a libwebkit.a libsqlite3.a libicui18n.a libicuuc.a libicudata.a libxml2.a libmodp_b64.a libxslt.a libjpeg.a libpcre.a libpng.a libz.a libharfbuzz.a libharfbuzz_interface.a
#CHROMIUMLIBS=$(addprefix $(CHROMIUMLIBPATH)/, $(CHROMIUMLIBNAMES))
CHROMIUMLIBS=-L$(CHROMIUMLIBPATH) -lsmime3 -lssl3 -lplds4 -lplc4 -lnspr4 -lpthread -ldl -lgdk-x11-2.0 -lgdk_pixbuf-2.0 -lm -lpangocairo-1.0 -lgio-2.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lfontconfig -lfreetype -lrt -lgconf-2 -lglib-2.0 -lX11 -lasound -lcommon -lbrowser -ldebugger -lrenderer -lutility -lprinting -lapp_base -lbase -licui18n -licuuc -licudata -levent -lbase_gfx -lskia -lharfbuzz -lharfbuzz_interface -ljpeg -lpng -lz -lnet -lgoogleurl -lsdch -lgtest -lbz2 -lmodp_b64 -lv8 -lv8_base -lxml2 -lglue -lwebcore -lpcre -lwtf -lxslt -lsqlite3 -lwebkit -lmedia -lffmpeg -lhunspell -lplugin

# Sadly you aren't allowed to link to executables directly this way.
#CHROMIUMLIBS= -Wl,-zmuldefs $(CHROMIUMDIR)/src/sconsbuild/Debug/chrome

CFLAGS= $(DEBUGFLAGS) -Wall -pthread -Wall -D_REENTRANT -D__STDC_FORMAT_MACROS -DCHROMIUM_BUILD -DU_STATIC_IMPLEMENTATION -g -m32 -I ../include `pkg-config --cflags gtk+-2.0 glib-2.0 gio-unix-2.0` $(addprefix -I$(CHROMIUMDIR)/src/,. third_party/npapi third_party/WebKit/JavaScriptCore third_party/icu38/public/common deps/third_party/icu42/public/common skia/config third_party/skia/include/core webkit/api/public third_party/WebKit/WebCore/platform/text)

LIBS= -g -m32 -Wl,--start-group `pkg-config --libs gtk+-2.0 glib-2.0 gio-unix-2.0 gconf-2.0` -lrt -lssl3 -lnspr4 -lnss3 -lnssutil3 -lbz2 $(CHROMIUMLIBS) -Wl,--end-group
#smime3

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)
HEADERS=$(wildcard ../include/*.h)

TARGET=../lib/libawesomium.so

#all: mymain

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir ../lib 2>/dev/null || true
	g++ $(OBJS) -shared $(LIBS) -o $@

%.o: %.cpp $(HEADERS)
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

mymain: $(OBJS)
	g++ $(CFLAGS) $(OBJS) $(LIBS) -o mymain

#libapp_base.a libbase.a libbase_gfx.a libbreakpad_client.a libbrowser.a libbz2.a libcommon.a libconvert_dict.a libdebugger.a libevent.a libffmpeg.a libglue.a libgmock.a libgmockmain.a libgoogleurl.a libgtest.a libgtestmain.a libharfbuzz.a libharfbuzz_interface.a libhunspell.a libicudata.a libicui18n.a libicuuc.a libjpeg.a liblzma_sdk.a libmedia.a libmodp_b64.a libnet.a libnet_test_support.a libpcre.a libplugin.a libpng.a libprinting.a librenderer.a libsdch.a libskia.a libsqlite3.a libtest_shell_common.a libtest_support_base.a libtest_support_common.a libtest_support_ui.a libtest_support_unit.a libutility.a libv8.a libv8_base.a libv8_nosnapshot.a libwebcore.a libwebkit.a libwtf.a libxml2.a libxslt.a libz.a

