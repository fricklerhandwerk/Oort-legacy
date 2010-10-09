LUAPKG:=$(shell pkg-config --exists luajit && echo luajit || echo lua)
PKGS:=glib-2.0 gtk+-2.0 gtkglext-1.0 gthread-2.0 $(LUAPKG)

CFLAGS:=`pkg-config --cflags $(PKGS)` -g -O2 -march=native -Wall -I.
LDFLAGS:=`pkg-config --libs $(PKGS)` -lGL -lGLU

common_sources = bullet.c  game.c  physics.c  scenario.c  ship.c  task.c team.c
common_objects = $(common_sources:.c=.o)

gl_sources = particle.o gl13.c glutil.c
gl_objects = $(gl_sources:.c=.o)

all: luacheck risc risc-dedicated

luacheck:
	@echo using Lua package $(LUAPKG)

%.d: %.c
				@set -e; rm -f $@; \
				$(CC) -M $(CFLAGS) $< > $@.$$$$; \
				sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
				rm -f $@.$$$$

-include $(common_sources:.c=.d)

%.c: %.vala vapi/risc.vapi
	valac -o $@ -C --pkg gtk+-2.0 --pkg gtkglext-1.0 --pkg risc --vapidir vapi $<

risc: risc.o $(gl_objects) $(common_objects)

risc-dedicated: risc-dedicated.o $(common_objects)

particlebench: particlebench.o $(gl_objects) $(common_objects)

clean:
	rm -f *.o *.d risc risc-dedicated particlebench
