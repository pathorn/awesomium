
CHROMIUMDIR=../../../chromium/chromium

CHROMIUMLIBNAMES=libbreakpad_client.a libbrowser.a libbz2.a libcommon.a libwebcore.a libconvert_dict.a libdebugger.a libgmock.a libgoogleurl.a libgtest.a libharfbuzz.a libharfbuzz_interface.a libhunspell.a libmedia.a libnet_test_support.a libplugin.a libprinting.a librenderer.a libtest_shell_common.a libtest_support_common.a libutility.a libglue.a libnet.a libsdch.a libv8.a libv8_base.a libv8.a libwebkit.a libwebcore.a libskia.a libglue.a libwebcore.a libapp_base.a libbase.a libbase_gfx.a libevent.a libffmpeg.a libwtf.a libwebkit.a libsqlite3.a libicui18n.a libicuuc.a libicudata.a libxml2.a libmodp_b64.a libxslt.a libjpeg.a liblzma_sdk.a libpcre.a libpng.a libz.a
CHROMIUMLIBS=$(addprefix $(CHROMIUMDIR)/src/sconsbuild/Debug/lib/, $(CHROMIUMLIBNAMES))

# Sadly you aren't allowed to link to executables directly this way.
#CHROMIUMLIBS= -Wl,-zmuldefs $(CHROMIUMDIR)/src/sconsbuild/Debug/chrome

CFLAGS= -g -m32 -I ../include `pkg-config --cflags gtk+-2.0 glib-2.0 gio-unix-2.0` $(addprefix -I$(CHROMIUMDIR)/src/,. third_party/npapi third_party/WebKit/JavaScriptCore skia/config third_party/skia/include/core webkit/api/public third_party/WebKit/WebCore/platform/text)

LIBS= -g -m32 $(CHROMIUMLIBS) `pkg-config --libs gtk+-2.0 glib-2.0 gio-unix-2.0 gconf-2.0` -lrt -lssl3 -lnspr4 -lnss3 -lnssutil3 -lbz2 
#smime3

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)
HEADERS=$(wildcard ../include/*.h)

TARGET=libawesomium.so

#all: mymain

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ $(OBJS) -shared $(LIBS) -o $@

%.o: %.cpp $(HEADERS)
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

mymain: $(OBJS)
	g++ $(CFLAGS) $(OBJS) $(LIBS) -o mymain

#libapp_base.a libbase.a libbase_gfx.a libbreakpad_client.a libbrowser.a libbz2.a libcommon.a libconvert_dict.a libdebugger.a libevent.a libffmpeg.a libglue.a libgmock.a libgmockmain.a libgoogleurl.a libgtest.a libgtestmain.a libharfbuzz.a libharfbuzz_interface.a libhunspell.a libicudata.a libicui18n.a libicuuc.a libjpeg.a liblzma_sdk.a libmedia.a libmodp_b64.a libnet.a libnet_test_support.a libpcre.a libplugin.a libpng.a libprinting.a librenderer.a libsdch.a libskia.a libsqlite3.a libtest_shell_common.a libtest_support_base.a libtest_support_common.a libtest_support_ui.a libtest_support_unit.a libutility.a libv8.a libv8_base.a libv8_nosnapshot.a libwebcore.a libwebkit.a libwtf.a libxml2.a libxslt.a libz.a

