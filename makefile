CC=g++
CFLAGS=-g3 -O0 -std=c++11 -fno-rtti -fPIC -DPOSIX -DJSONCPP_RELATIVE_PATH \
	-I/home/mburakov/webrtc/trunk \
	-I/home/mburakov/webrtc/trunk/third_party/jsoncpp/source/include \
	-I/usr/include/qt
LDFLAGS=-L/home/mburakov/webrtc/trunk/out/Release -lWebRTC -lpthread -ldl -lQt5Widgets -lQt5Core -lQt5Gui
LIBS=nspr nss x11 expat xext

CFLAGS+=$(shell pkg-config --cflags $(LIBS))
LDFLAGS+=$(shell pkg-config --libs $(LIBS))
SOURCES=$(wildcard linux/*.cc) $(wildcard *.cc)
RESULT=$(notdir $(CURDIR))
COMPLETION=.clang_complete

all: $(COMPLETION) $(RESULT)

$(COMPLETION):
	echo $(CFLAGS) > $(COMPLETION)

$(RESULT): $(SOURCES)
	moc main_wnd.h > main_wnd_moc.cc
	$(CC) $(CFLAGS) $(SOURCES) -o $(RESULT) $(LDFLAGS)

clean:
	rm $(RESULT) $(COMPLETION) main_wnd_moc.cc

.PHONY: all clean
