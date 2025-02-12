CC = em++
CFLAGS_COMMON = -Iinclude -I3rd -I3rd/multiprecision-Boost_1_86_0/include -I3rd/md5-c --std=c++20 
CFLAGS = -O3 -Wall -lembind --emit-tsd interface.d.ts $(CFLAGS_COMMON) -sASYNCIFY --post-js=mixin.js
TARGET = blufi.mjs
OUTDIR = build

all: $(OUTDIR)/$(TARGET)

$(OUTDIR)/$(TARGET): wasm.cpp | $(OUTDIR)
	$(CC) $(CFLAGS) -o $(OUTDIR)/$(TARGET) wasm.cpp blufi.cpp dh.cpp msg.cpp 3rd/uaes/uaes.c 3rd/md5-c/md5.c

$(OUTDIR):
	mkdir -p $(OUTDIR)

clean:
	rm -rf $(OUTDIR)

test: $(OUTDIR)
	g++ -o $(OUTDIR)/test test.cpp msg.cpp 3rd/uaes/uaes.c $(CFLAGS_COMMON) && $(OUTDIR)/test