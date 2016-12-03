VERSION = 0.1

HEADERS = ../sss7core/sss7.h libsss7.h
OBJDIR = bin

CC = clang

CFLAGS =  -DSSS7_THREADED -I ../sss7core/ -Os -Wall -Wstrict-prototypes -fPIC
LDFLAGS = -pthread -Wl,--gc-sections

all: start $(OBJDIR)/libsss7.so $(OBJDIR)/test
	@echo ":: Done !"

start:
	@echo " SSS7 linux port $(VERSION)"
	@echo "=========================="

$(OBJDIR)/test : $(OBJDIR)/libsss7.so

$(OBJDIR)/%.o : %.c $(HEADERS) Makefile
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/sss7core/%.o : ../sss7core/%.c $(HEADERS) Makefile
		@mkdir -p $$(dirname $@)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/libsss7.so : $(OBJDIR)/libsss7.o $(OBJDIR)/sss7core/sss7.o
		@mkdir -p $$(dirname $@)
		$(CC) $+ -shared $(LDFLAGS) -o $@

$(OBJDIR)/$(TARGET) : $(TARGET).o
	$(CC) $(LDFLAGS) $+ -o $@


clean :
	@rm -rf $(OBJDIR)

test: all
	$(OBJDIR)/test
