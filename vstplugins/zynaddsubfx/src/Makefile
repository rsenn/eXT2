include Makefile.inc

#ifneq ($(MAKECMDGOALS),debug)
#  CXXFLAGS= -O2 -ggdb -Wall
#else
#  CXXFLAGS= -O0 -ggdb -Wall -Wpointer-arith -Wstrict-prototypes
#endif

CXXFLAGS= -DOS_$(OS_PORT) -D$(MIDIIN)MIDIIN -D$(AUDIOOUT)AUDIOOUT -DFFTW_VERSION_$(FFTW_VERSION) -DASM_F2I_$(ASM_F2I) -I"../../fltk" -I"../fltk" -march=i686 -pipe -O2 -fomit-frame-pointer
export CXXFLAGS

LIBS= -L"/usr/lib" -lX11 -lXft -lXext -lfftw3 -lmxml -lm -lz -lpthread

objects=main.o
SUBDIRS=DSP Effects Input Misc Output Params Synth Seq

.PHONY: subdirs $(SUBDIRS)

all:
	$(MAKE) -C UI $@
	@for name in $(SUBDIRS); do sh -c "cd $$name ; $(CXX) -MM -MG -w *.C >> ../Make.deps ; cd .."; done
	$(MAKE) subdirs
	$(MAKE) objs
	rm -f zynaddsubfx zynaddsubfx.exe
	rm -f Make.deps


ifneq ($(AUDIOOUT),VST)
#	$(CXX) -o zynaddsubfx */*.o *.o $(LIBS)
else
	g++  -shared -fPIC -ggdb -o zynaddsubfx_vst.so */*.o *.o ../fltk/lib/libfltk.a $(LIBS)
endif

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

objs:$(objects)

debug: all

main.o:Misc/Master.h Misc/Util.h Output/OSSaudiooutput.h\
       Input/OSSMidiIn.h Input/ALSAMidiIn.h


.PHONY : clean
clean:
	rm -f $(objects) zynaddsubfx zynaddsubfx_vst.so zynaddsubfx.exe
	@for name in $(SUBDIRS); do sh -c "make -C $$name $@"; done
	rm -f Make.deps
	rm -f */*.o *.o
	$(MAKE) -C UI $@

